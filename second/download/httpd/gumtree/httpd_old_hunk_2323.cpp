    }

#ifdef LDAP_OPT_NETWORK_TIMEOUT
    st->connectionTimeout = atol(ttl);

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, cmd->server,
                 "[%" APR_PID_T_FMT "] ldap connection: Setting connection"
                 " timeout to %ld seconds.", getpid(), st->connectionTimeout);
#else
    ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, cmd->server,
                 "LDAP: Connection timeout option not supported by the "
                 "LDAP SDK in use." );
#endif

