ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
                         "MinSpareThreads of %d not allowed, increasing to 1",
                         ap_threads_min_free);