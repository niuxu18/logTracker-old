ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r,
                              "auth_ldap authorize: require ldap-filter: "
                              "%s authorization failed [%s][%s]",
                              filtbuf, ldc->reason, ldap_err2string(result));