ap_log_error(APLOG_MARK, APLOG_ERR, rv, s,
                         "Cannot reinit %s mutex with file `%s'",
                         SSL_CACHE_MUTEX_TYPE, lockfile);