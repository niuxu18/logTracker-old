ap_log_error(APLOG_MARK, APLOG_CRIT, apr_get_os_error(), ap_server_conf,
                         "Child %lu: Failed to create a qwait event.", my_pid);