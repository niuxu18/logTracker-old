    apr_socket_t *newsock;
    void *sconf = s->module_config;
    proxy_server_conf *conf =
        (proxy_server_conf *) ap_get_module_config(sconf, &proxy_module);

    if (conn->sock) {
        if (!(connected = is_socket_connected(conn->sock))) {
            socket_cleanup(conn);
            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s,
                         "proxy: %s: backend socket is disconnected.",
                         proxy_function);
        }
    }
    while (backend_addr && !connected) {
        if ((rv = apr_socket_create(&newsock, backend_addr->family,
                                SOCK_STREAM, APR_PROTO_TCP,
                                conn->scpool)) != APR_SUCCESS) {
            loglevel = backend_addr->next ? APLOG_DEBUG : APLOG_ERR;
            ap_log_error(APLOG_MARK, loglevel, rv, s,
                         "proxy: %s: error creating fam %d socket for target %s",
                         proxy_function,
                         backend_addr->family,
                         worker->hostname);
