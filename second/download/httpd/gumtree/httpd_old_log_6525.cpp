ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r, APLOGNO(02473) 
                "cache: %s responded with an uncacheable 304, " 
                "retrying the request. Reason: %s", 
                r->unparsed_uri, reason);