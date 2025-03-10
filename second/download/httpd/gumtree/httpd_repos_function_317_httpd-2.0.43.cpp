static int dav_method_put(request_rec *r)
{
    dav_resource *resource;
    int resource_state;
    dav_auto_version_info av_info;
    const dav_hooks_locks *locks_hooks = DAV_GET_HOOKS_LOCKS(r);
    const char *body;
    dav_error *err;
    dav_error *err2;
    int result;
    dav_stream_mode mode;
    dav_stream *stream;
    dav_response *multi_response;
    int has_range;
    apr_off_t range_start;
    apr_off_t range_end;

    if ((result = ap_setup_client_block(r, REQUEST_CHUNKED_DECHUNK)) != OK) {
        return result;
    }

    /* Ask repository module to resolve the resource */
    err = dav_get_resource(r, 0 /* label_allowed */, 0 /* use_checked_in */,
                           &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);

    /* If not a file or collection resource, PUT not allowed */
    if (resource->type != DAV_RESOURCE_TYPE_REGULAR
        && resource->type != DAV_RESOURCE_TYPE_WORKING) {
        body = apr_psprintf(r->pool,
                            "Cannot create resource %s with PUT.",
                            ap_escape_html(r->pool, r->uri));
        return dav_error_response(r, HTTP_CONFLICT, body);
    }

    /* Cannot PUT a collection */
    if (resource->collection) {
        return dav_error_response(r, HTTP_CONFLICT,
                                  "Cannot PUT to a collection.");

    }

    resource_state = dav_get_resource_state(r, resource);

    /*
     * Note: depth == 0 normally requires no multistatus response. However,
     * if we pass DAV_VALIDATE_PARENT, then we could get an error on a URI
     * other than the Request-URI, thereby requiring a multistatus.
     *
     * If the resource does not exist (DAV_RESOURCE_NULL), then we must
     * check the resource *and* its parent. If the resource exists or is
     * a locknull resource, then we check only the resource.
     */
    if ((err = dav_validate_request(r, resource, 0, NULL, &multi_response,
                                    resource_state == DAV_RESOURCE_NULL ?
                                    DAV_VALIDATE_PARENT :
                                    DAV_VALIDATE_RESOURCE, NULL)) != NULL) {
        /* ### add a higher-level description? */
        return dav_handle_err(r, err, multi_response);
    }

    /* make sure the resource can be modified (if versioning repository) */
    if ((err = dav_auto_checkout(r, resource,
                                 0 /* not parent_only */,
                                 &av_info)) != NULL) {
        /* ### add a higher-level description? */
        return dav_handle_err(r, err, NULL);
    }

    /* truncate and rewrite the file unless we see a Content-Range */
    mode = DAV_MODE_WRITE_TRUNC;

    has_range = dav_parse_range(r, &range_start, &range_end);
    if (has_range) {
        mode = DAV_MODE_WRITE_SEEKABLE;
    }

    /* Create the new file in the repository */
    if ((err = (*resource->hooks->open_stream)(resource, mode,
                                               &stream)) != NULL) {
        /* ### assuming FORBIDDEN is probably not quite right... */
        err = dav_push_error(r->pool, HTTP_FORBIDDEN, 0,
                             apr_psprintf(r->pool,
                                          "Unable to PUT new contents for %s.",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
    }

    if (err == NULL && has_range) {
        /* a range was provided. seek to the start */
        err = (*resource->hooks->seek_stream)(stream, range_start);
    }

    if (err == NULL) {
        if (ap_should_client_block(r)) {
            char *buffer = apr_palloc(r->pool, DAV_READ_BLOCKSIZE);
            long len;

            /*
             * Once we start reading the request, then we must read the
             * whole darn thing. ap_discard_request_body() won't do anything
             * for a partially-read request.
             */

            while ((len = ap_get_client_block(r, buffer,
                                              DAV_READ_BLOCKSIZE)) > 0) {
                   if (err == NULL) {
                       /* write whatever we read, until we see an error */
                       err = (*resource->hooks->write_stream)(stream,
                                                              buffer, len);
                   }
            }

            /*
             * ### what happens if we read more/less than the amount
             * ### specified in the Content-Range? eek...
             */

            if (len == -1) {
                /*
                 * Error reading request body. This has precedence over
                 * prior errors.
                 */
                err = dav_new_error(r->pool, HTTP_BAD_REQUEST, 0,
                                    "An error occurred while reading the "
                                    "request body.");
            }
        }

        err2 = (*resource->hooks->close_stream)(stream,
                                                err == NULL /* commit */);
        if (err2 != NULL && err == NULL) {
            /* no error during the write, but we hit one at close. use it. */
            err = err2;
        }
    }

    /*
     * Ensure that we think the resource exists now.
     * ### eek. if an error occurred during the write and we did not commit,
     * ### then the resource might NOT exist (e.g. dav_fs_repos.c)
     */
    if (err == NULL) {
        resource->exists = 1;
    }

    /* restore modifiability of resources back to what they were */
    err2 = dav_auto_checkin(r, resource, err != NULL /* undo if error */,
                            0 /*unlock*/, &av_info);

    /* check for errors now */
    if (err != NULL) {
        return dav_handle_err(r, err, NULL);
    }

    if (err2 != NULL) {
        /* just log a warning */
        err2 = dav_push_error(r->pool, err->status, 0,
                              "The PUT was successful, but there "
                              "was a problem automatically checking in "
                              "the resource or its parent collection.",
                              err2);
        dav_log_err(r, err2, APLOG_WARNING);
    }

    /* ### place the Content-Type and Content-Language into the propdb */

    if (locks_hooks != NULL) {
        dav_lockdb *lockdb;

        if ((err = (*locks_hooks->open_lockdb)(r, 0, 0, &lockdb)) != NULL) {
            /* The file creation was successful, but the locking failed. */
            err = dav_push_error(r->pool, err->status, 0,
                                 "The file was PUT successfully, but there "
                                 "was a problem opening the lock database "
                                 "which prevents inheriting locks from the "
                                 "parent resources.",
                                 err);
            return dav_handle_err(r, err, NULL);
        }

        /* notify lock system that we have created/replaced a resource */
        err = dav_notify_created(r, lockdb, resource, resource_state, 0);

        (*locks_hooks->close_lockdb)(lockdb);

        if (err != NULL) {
            /* The file creation was successful, but the locking failed. */
            err = dav_push_error(r->pool, err->status, 0,
                                 "The file was PUT successfully, but there "
                                 "was a problem updating its lock "
                                 "information.",
                                 err);
            return dav_handle_err(r, err, NULL);
        }
    }

    /* NOTE: WebDAV spec, S8.7.1 states properties should be unaffected */

    /* return an appropriate response (HTTP_CREATED or HTTP_NO_CONTENT) */
    return dav_created(r, NULL, "Resource", resource_state == DAV_RESOURCE_EXISTS);
}