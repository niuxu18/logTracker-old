ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r,
                              "Request header exceeds LimitRequestFieldSize "
                              "after folding: %.*s",
                              field_name_len(last_field), last_field);