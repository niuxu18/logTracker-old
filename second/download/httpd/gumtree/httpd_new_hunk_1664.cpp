                     "proxy: BALANCER: (%s). Unlock failed for pre_request",
                     (*balancer)->name);
    }
    if (!*worker) {
        runtime = find_best_worker(*balancer, r);
        if (!runtime) {
            if ((*balancer)->workers->nelts) {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                            "proxy: BALANCER: (%s). All workers are in error state",
                            (*balancer)->name);
            } else {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                            "proxy: BALANCER: (%s). No workers in balancer",
                            (*balancer)->name);
            }

            return HTTP_SERVICE_UNAVAILABLE;
        }
        if ((*balancer)->sticky && runtime) {
            /*
             * This balancer has sticky sessions and the client either has not
