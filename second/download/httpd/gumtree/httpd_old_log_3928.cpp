ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, cmd->server,
                 "LDAP cache: Setting shared memory cache file to %s bytes.",
                 st->cache_file);