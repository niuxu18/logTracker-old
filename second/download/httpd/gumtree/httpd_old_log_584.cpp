ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                  "Digest: access to %s failed, reason: user %s not "
                  "allowed access", r->uri, user);