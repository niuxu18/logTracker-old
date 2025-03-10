ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r,
                  "[%" APR_PID_T_FMT "] auth_ldap authenticate: "
                  "REMOTE_USER was to be set with attribute '%s', "
                  "but this attribute was not requested for in the "
                  "LDAP query for the user. REMOTE_USER will fall "
                  "back to username or DN as appropriate.", getpid(),
                  sec->remote_user_attribute);