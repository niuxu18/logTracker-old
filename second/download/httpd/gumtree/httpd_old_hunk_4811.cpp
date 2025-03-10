        if (rc != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_ERR, rc, ap_server_conf,
                         "process_socket: apr_pollset_add failure");
            AP_DEBUG_ASSERT(rc == APR_SUCCESS);
        }
    }
    return 1;
}

/* requests_this_child has gone to zero or below.  See if the admin coded
   "MaxConnectionsPerChild 0", and keep going in that case.  Doing it this way
   simplifies the hot path in worker_thread */
static void check_infinite_requests(void)
{
    if (ap_max_requests_per_child) {
        signal_threads(ST_GRACEFUL);
    }
    else {
        requests_this_child = INT_MAX;  /* keep going */
    }
}

static void close_listeners(int process_slot, int *closed) {
    if (!*closed) {
        int i;
