ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00443)
                         "changing ThreadLimit to %d from original value "
                         "of %d not allowed during restart",
                         thread_limit, first_thread_limit);