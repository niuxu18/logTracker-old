ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
                         "ThreadsPerChild of %d not allowed, increasing to 1",
                         ap_threads_per_child);