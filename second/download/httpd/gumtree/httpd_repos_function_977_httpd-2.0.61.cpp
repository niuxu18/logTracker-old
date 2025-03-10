static int hook_uri2file(request_rec *r)
{
    rewrite_server_conf *conf;
    const char *saved_rulestatus;
    const char *var;
    const char *thisserver;
    char *thisport;
    const char *thisurl;
    char buf[512];
    char docroot[512];
    const char *ccp;
    unsigned int port;
    int rulestatus;
    int n;
    int l;

    /*
     *  retrieve the config structures
     */
    conf = ap_get_module_config(r->server->module_config, &rewrite_module);

    /*
     *  only do something under runtime if the engine is really enabled,
     *  else return immediately!
     */
    if (conf->state == ENGINE_DISABLED) {
        return DECLINED;
    }

    /*
     *  check for the ugly API case of a virtual host section where no
     *  mod_rewrite directives exists. In this situation we became no chance
     *  by the API to setup our default per-server config so we have to
     *  on-the-fly assume we have the default config. But because the default
     *  config has a disabled rewriting engine we are lucky because can
     *  just stop operating now.
     */
    if (conf->server != r->server) {
        return DECLINED;
    }

    /*
     *  add the SCRIPT_URL variable to the env. this is a bit complicated
     *  due to the fact that apache uses subrequests and internal redirects
     */

    if (r->main == NULL) {
         var = apr_pstrcat(r->pool, "REDIRECT_", ENVVAR_SCRIPT_URL, NULL);
         var = apr_table_get(r->subprocess_env, var);
         if (var == NULL) {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, r->uri);
         }
         else {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
         }
    }
    else {
         var = apr_table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
    }

    /*
     *  create the SCRIPT_URI variable for the env
     */

    /* add the canonical URI of this URL */
    thisserver = ap_get_server_name(r);
    port = ap_get_server_port(r);
    if (ap_is_default_port(port, r)) {
        thisport = "";
    }
    else {
        apr_snprintf(buf, sizeof(buf), ":%u", port);
        thisport = buf;
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);

    /* set the variable */
    var = apr_pstrcat(r->pool, ap_http_method(r), "://", thisserver, thisport,
                      thisurl, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);

    if (!(saved_rulestatus = apr_table_get(r->notes,"mod_rewrite_rewritten"))) {
        /* if filename was not initially set,
         * we start with the requested URI
         */
        if (r->filename == NULL) {
            r->filename = apr_pstrdup(r->pool, r->uri);
            rewritelog(r, 2, "init rewrite engine with requested uri %s",
                       r->filename);
        }
        else {
            rewritelog(r, 2, "init rewrite engine with passed filename %s."
                       " Original uri = %s", r->filename, r->uri);
        }

        /*
         *  now apply the rules ...
         */
        rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
        apr_table_set(r->notes,"mod_rewrite_rewritten",
                      apr_psprintf(r->pool,"%d",rulestatus));
    }
    else {
        rewritelog(r, 2,
                   "uri already rewritten. Status %s, Uri %s, r->filename %s",
                   saved_rulestatus, r->uri, r->filename);
        rulestatus = atoi(saved_rulestatus);
    }

    if (rulestatus) {
        unsigned skip;

        if (strlen(r->filename) > 6 &&
            strncmp(r->filename, "proxy:", 6) == 0) {
            /* it should be go on as an internal proxy request */

            /* check if the proxy module is enabled, so
             * we can actually use it!
             */
            if (!proxy_available) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                              "attempt to make remote request from mod_rewrite "
                              "without proxy enabled: %s", r->filename);
                return HTTP_FORBIDDEN;
            }

            /* make sure the QUERY_STRING and
             * PATH_INFO parts get incorporated
             */
            if (r->path_info != NULL) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          r->path_info, NULL);
            }
            if (r->args != NULL &&
                r->uri == r->unparsed_uri) {
                /* see proxy_http:proxy_http_canon() */
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          "?", r->args, NULL);
            }

            /* now make sure the request gets handled by the proxy handler */
            if (PROXYREQ_NONE == r->proxyreq) {
                r->proxyreq = PROXYREQ_REVERSE;
            }
            r->handler  = "proxy-server";

            rewritelog(r, 1, "go-ahead with proxy request %s [OK]",
                       r->filename);
            return OK;
        }
        else if ((skip = is_absolute_uri(r->filename)) > 0) {
            /* it was finally rewritten to a remote URL */

            if (rulestatus != ACTION_NOESCAPE) {
                rewritelog(r, 1, "escaping %s for redirect", r->filename);
                r->filename = escape_absolute_uri(r->pool, r->filename, skip);
            }

            /* append the QUERY_STRING part */
            if (r->args) {
                r->filename = apr_pstrcat(r->pool, r->filename, "?",
                                          (rulestatus == ACTION_NOESCAPE)
                                            ? r->args
                                            : ap_escape_uri(r->pool, r->args),
                                          NULL);
            }

            /* determine HTTP redirect response code */
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK; /* make Apache kernel happy */
            }
            else {
                n = HTTP_MOVED_TEMPORARILY;
            }

            /* now do the redirection */
            apr_table_setn(r->headers_out, "Location", r->filename);
            rewritelog(r, 1, "redirect to %s [REDIRECT/%d]", r->filename, n);
            return n;
        }
        else if (strlen(r->filename) > 10 &&
                 strncmp(r->filename, "forbidden:", 10) == 0) {
            /* This URLs is forced to be forbidden for the requester */
            return HTTP_FORBIDDEN;
        }
        else if (strlen(r->filename) > 5 &&
                 strncmp(r->filename, "gone:", 5) == 0) {
            /* This URLs is forced to be gone */
            return HTTP_GONE;
        }
        else if (strlen(r->filename) > 12 &&
                 strncmp(r->filename, "passthrough:", 12) == 0) {
            /*
             * Hack because of underpowered API: passing the current
             * rewritten filename through to other URL-to-filename handlers
             * just as it were the requested URL. This is to enable
             * post-processing by mod_alias, etc.  which always act on
             * r->uri! The difference here is: We do not try to
             * add the document root
             */
            r->uri = apr_pstrdup(r->pool, r->filename+12);
            return DECLINED;
        }
        else {
            /* it was finally rewritten to a local path */

            /* expand "/~user" prefix */
#if APR_HAS_USER
            r->filename = expand_tildepaths(r, r->filename);
#endif
            rewritelog(r, 2, "local path result: %s", r->filename);

            /* the filename must be either an absolute local path or an
             * absolute local URL.
             */
            if (   *r->filename != '/'
                && !ap_os_is_path_absolute(r->pool, r->filename)) {
                return HTTP_BAD_REQUEST;
            }

            /* if there is no valid prefix, we have
             * to emulate the translator from the core and
             * prefix the filename with document_root
             *
             * NOTICE:
             * We cannot leave out the prefix_stat because
             * - when we always prefix with document_root
             *   then no absolute path can be created, e.g. via
             *   emulating a ScriptAlias directive, etc.
             * - when we always NOT prefix with document_root
             *   then the files under document_root have to
             *   be references directly and document_root
             *   gets never used and will be a dummy parameter -
             *   this is also bad
             *
             * BUT:
             * Under real Unix systems this is no problem,
             * because we only do stat() on the first directory
             * and this gets cached by the kernel for along time!
             */
            n = prefix_stat(r->filename, r->pool);
            if (n == 0) {
                if ((ccp = ap_document_root(r)) != NULL) {
                    l = apr_cpystrn(docroot, ccp, sizeof(docroot)) - docroot;

                    /* always NOT have a trailing slash */
                    if (docroot[l-1] == '/') {
                        docroot[l-1] = '\0';
                    }
                    if (r->server->path
                        && !strncmp(r->filename, r->server->path,
                                    r->server->pathlen)) {
                        r->filename = apr_pstrcat(r->pool, docroot,
                                                  (r->filename +
                                                   r->server->pathlen), NULL);
                    }
                    else {
                        r->filename = apr_pstrcat(r->pool, docroot,
                                                  r->filename, NULL);
                    }
                    rewritelog(r, 2, "prefixed with document_root to %s",
                               r->filename);
                }
            }

            rewritelog(r, 1, "go-ahead with %s [OK]", r->filename);
            return OK;
        }
    }
    else {
        rewritelog(r, 1, "pass through %s", r->filename);
        return DECLINED;
    }
}