ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r, 
                      "Error while reading HTTP trailer: %i%s%s",
                      r->status, error_notes ? ": " : "",
                      error_notes ? error_notes : "");