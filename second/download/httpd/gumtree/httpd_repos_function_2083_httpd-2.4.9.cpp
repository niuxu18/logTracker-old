static int proxy_fcgi_handler(request_rec *r, proxy_worker *worker,
                              proxy_server_conf *conf,
                              char *url, const char *proxyname,
                              apr_port_t proxyport)
{
    int status;
    char server_portstr[32];
    conn_rec *origin = NULL;
    proxy_conn_rec *backend;

    proxy_dir_conf *dconf = ap_get_module_config(r->per_dir_config,
                                                 &proxy_module);

    apr_pool_t *p = r->pool;

    apr_uri_t *uri = apr_palloc(r->pool, sizeof(*uri));

    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01076)
                  "url: %s proxyname: %s proxyport: %d",
                 url, proxyname, proxyport);

    if (strncasecmp(url, "fcgi:", 5) != 0) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01077) "declining URL %s", url);
        return DECLINED;
    }

    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01078) "serving URL %s", url);

    /* Create space for state information */
    status = ap_proxy_acquire_connection(FCGI_SCHEME, &backend, worker,
                                         r->server);
    if (status != OK) {
        if (backend) {
            backend->close = 1;
            ap_proxy_release_connection(FCGI_SCHEME, backend, r->server);
        }
        return status;
    }

    backend->is_ssl = 0;

    /* Step One: Determine Who To Connect To */
    status = ap_proxy_determine_connection(p, r, conf, worker, backend,
                                           uri, &url, proxyname, proxyport,
                                           server_portstr,
                                           sizeof(server_portstr));
    if (status != OK) {
        goto cleanup;
    }

    /* XXX Setting close to 0 is a great way to end up with
     *     timeouts at this point, since we lack good ways to manage the
     *     back end fastcgi processes.  This should be revisited when we
     *     have a better story on that part of things. */
    backend->close = 1;

    /* Step Two: Make the Connection */
    if (ap_proxy_connect_backend(FCGI_SCHEME, backend, worker, r->server)) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01079)
                      "failed to make connection to backend: %s",
                      backend->hostname);
        status = HTTP_SERVICE_UNAVAILABLE;
        goto cleanup;
    }

    /* Step Three: Process the Request */
    status = fcgi_do_request(p, r, backend, origin, dconf, uri, url,
                             server_portstr);

cleanup:
    ap_proxy_release_connection(FCGI_SCHEME, backend, r->server);
    return status;
}