ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, f->r,
                              "Unknown Transfer-Encoding: %s; "
                              "using read-until-close", tenc);