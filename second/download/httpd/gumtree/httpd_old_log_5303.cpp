ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL,
                         "WARNING: ThreadsPerChild of %d not allowed, "
                         "increasing to 1.", ap_threads_per_child);