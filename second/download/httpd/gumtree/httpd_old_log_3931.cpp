ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, cmd->server,
                 "ldap cache: Setting operation cache TTL to %ld microseconds.",
                 st->compare_cache_ttl);