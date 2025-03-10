    hb_server_t *server;
    apr_array_header_t *up_servers;
    proxy_worker *mycandidate = NULL;
    apr_pool_t *tpool;
    apr_hash_t *servers;

    lb_hb_ctx_t *ctx =
        ap_get_module_config(r->server->module_config,
                             &lbmethod_heartbeat_module);

    if (!ap_proxy_retry_worker_fn) {
        ap_proxy_retry_worker_fn =
                APR_RETRIEVE_OPTIONAL_FN(ap_proxy_retry_worker);
        if (!ap_proxy_retry_worker_fn) {
            /* can only happen if mod_proxy isn't loaded */
            return NULL;
        }
    }

    apr_pool_create(&tpool, r->pool);

    servers = apr_hash_make(tpool);

    rv = read_heartbeats(ctx->path, servers, tpool);

    if (rv) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01213)
                      "lb_heartbeat: Unable to read heartbeats at '%s'",
                      ctx->path);
        apr_pool_destroy(tpool);
        return NULL;
    }

    up_servers = apr_array_make(tpool, apr_hash_count(servers), sizeof(hb_server_t *));

    for (i = 0; i < balancer->workers->nelts; i++) {
        worker = &APR_ARRAY_IDX(balancer->workers, i, proxy_worker *);
        server = apr_hash_get(servers, (*worker)->s->hostname, APR_HASH_KEY_STRING);

        if (!server) {
            ap_log_rerror(APLOG_MARK, APLOG_DEBUG, rv, r, APLOGNO(01214)
                      "lb_heartbeat: No server for worker %s", (*worker)->s->name);
            continue;
        }

        if (!PROXY_WORKER_IS_USABLE(*worker)) {
            ap_proxy_retry_worker_fn("BALANCER", *worker, r->server);
        }

        if (PROXY_WORKER_IS_USABLE(*worker)) {
            server->worker = *worker;
            if (server->seen < LBM_HEARTBEAT_MAX_LASTSEEN) {
                openslots += server->ready;
