ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(02900)
                          "declining URL %s  (not %s, Upgrade: header is %s)", 
                          url, upgrade_method, upgrade ? upgrade : "missing");