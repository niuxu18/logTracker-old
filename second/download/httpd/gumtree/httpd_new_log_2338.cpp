ap_log_error(APLOG_MARK, level, rv, ap_server_conf,
                 "apr_proc_mutex_%s failed. Attempting to "
                 "shutdown process gracefully.", func);