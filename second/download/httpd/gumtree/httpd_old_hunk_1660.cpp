        return DECLINED;
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                 "proxy: AJP: serving URL %s", url);

    /* create space for state information */
    if (!backend) {
        status = ap_proxy_acquire_connection(scheme, &backend, worker,
                                             r->server);
        if (status != OK) {
            if (backend) {
                backend->close_on_recycle = 1;
                ap_proxy_release_connection(scheme, backend, r->server);
            }
            return status;
        }
    }

    backend->is_ssl = 0;
    backend->close_on_recycle = 0;

    retry = 0;
    while (retry < 2) {
        char *locurl = url;
        /* Step One: Determine Who To Connect To */
        status = ap_proxy_determine_connection(p, r, conf, worker, backend,
