ap_log_rerror(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, r, APLOGNO(01626)
                  "authorization result of %s: %s",
                  format_authz_command(r->pool, section),
                  format_authz_result(auth_result));