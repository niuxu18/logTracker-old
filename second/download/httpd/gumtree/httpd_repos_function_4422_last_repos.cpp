static int netware_check_config(apr_pool_t *p, apr_pool_t *plog,
                                apr_pool_t *ptemp, server_rec *s)
{
    static int restart_num = 0;
    int startup = 0;

    /* we want this only the first time around */
    if (restart_num++ == 0) {
        startup = 1;
    }

    if (ap_threads_limit > HARD_THREAD_LIMIT) {
        if (startup) {
            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00228)
                         "WARNING: MaxThreads of %d exceeds compile-time "
                         "limit of %d threads, decreasing to %d. "
                         "To increase, please see the HARD_THREAD_LIMIT "
                         "define in server/mpm/netware%s.",
                         ap_threads_limit, HARD_THREAD_LIMIT, HARD_THREAD_LIMIT,
                         MPM_HARD_LIMITS_FILE);
        } else {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00229)
                         "MaxThreads of %d exceeds compile-time limit "
                         "of %d, decreasing to match",
                         ap_threads_limit, HARD_THREAD_LIMIT);
        }
        ap_threads_limit = HARD_THREAD_LIMIT;
    }
    else if (ap_threads_limit < 1) {
        if (startup) {
            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00230)
                         "WARNING: MaxThreads of %d not allowed, "
                         "increasing to 1.", ap_threads_limit);
        } else {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(02661)
                         "MaxThreads of %d not allowed, increasing to 1",
                         ap_threads_limit);
        }
        ap_threads_limit = 1;
    }

    /* ap_threads_to_start > ap_threads_limit effectively checked in
     * call to startup_workers(ap_threads_to_start) in ap_mpm_run()
     */
    if (ap_threads_to_start < 0) {
        if (startup) {
            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00231)
                         "WARNING: StartThreads of %d not allowed, "
                         "increasing to 1.", ap_threads_to_start);
        } else {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00232)
                         "StartThreads of %d not allowed, increasing to 1",
                         ap_threads_to_start);
        }
        ap_threads_to_start = 1;
    }

    if (ap_threads_min_free < 1) {
        if (startup) {
            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00233)
                         "WARNING: MinSpareThreads of %d not allowed, "
                         "increasing to 1 to avoid almost certain server failure. "
                         "Please read the documentation.", ap_threads_min_free);
        } else {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00234)
                         "MinSpareThreads of %d not allowed, increasing to 1",
                         ap_threads_min_free);
        }
        ap_threads_min_free = 1;
    }

    /* ap_threads_max_free < ap_threads_min_free + 1 checked in ap_mpm_run() */

    return OK;
}