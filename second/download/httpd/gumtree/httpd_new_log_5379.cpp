ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00304)
                     "changing ServerLimit to %d from original value of %d "
                     "not allowed during restart",
                     server_limit, retained->first_server_limit);