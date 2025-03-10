        thisport = buf;
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);

    /* set the variable */
    var = apr_pstrcat(r->pool, ap_http_method(r), "://", thisserver, thisport,
                     thisurl, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);

    /* if filename was not initially set,
     * we start with the requested URI
     */
    if (r->filename == NULL) {
        r->filename = apr_pstrdup(r->pool, r->uri);
        rewritelog(r, 2, "init rewrite engine with requested uri %s",
                   r->filename);
    }

    /*
     *  now apply the rules ...
     */
    rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
    if (rulestatus) {

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
            r->proxyreq = PROXYREQ_REVERSE;
            r->handler  = "proxy-server";

            rewritelog(r, 1, "go-ahead with proxy request %s [OK]",
                       r->filename);
            return OK;
        }
        else if (is_absolute_uri(r->filename)) {
            /* it was finally rewritten to a remote URL */

            /* skip 'scheme:' */
            for (cp = r->filename; *cp != ':' && *cp != '\0'; cp++)
                ;
            /* skip '://' */
            cp += 3;
            /* skip host part */
            for ( ; *cp != '/' && *cp != '\0'; cp++)
                ;
            if (*cp != '\0') {
                if (rulestatus != ACTION_NOESCAPE) {
                    rewritelog(r, 1, "escaping %s for redirect", r->filename);
                    cp2 = ap_escape_uri(r->pool, cp);
                }
                else {
                    cp2 = apr_pstrdup(r->pool, cp);
                }
                *cp = '\0';
                r->filename = apr_pstrcat(r->pool, r->filename, cp2, NULL);
            }

            /* append the QUERY_STRING part */
            if (r->args != NULL) {
                char *args;
                if (rulestatus == ACTION_NOESCAPE) {
                    args = r->args;
                }
                else {
                    args = ap_escape_uri(r->pool, r->args);
                }
                r->filename = apr_pstrcat(r->pool, r->filename, "?", 
                                          args, NULL);
            }

            /* determine HTTP redirect response code */
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK; /* make Apache kernel happy */
