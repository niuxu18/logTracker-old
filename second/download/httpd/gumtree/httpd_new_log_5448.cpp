ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r, APLOGNO(00026)
                                  "found %%2f (encoded '/') in URI "
                                  "(decoded='%s'), returning 404",
                                  r->parsed_uri.path);