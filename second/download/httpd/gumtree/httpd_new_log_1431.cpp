ap_log_error(APLOG_MARK,APLOG_ERR, status, ap_server_conf,
                     "Child %lu: Failed to acquire the start_mutex. Process will exit.", my_pid);