            apr_table_setn(r->headers_out, "Location",
                          ap_construct_url(r->pool, nuri, r));

            ap_log_rerror(APLOG_MARK, APLOG_INFO, APR_SUCCESS,
                          r,
                          ref ? "Fixed spelling: %s to %s from %s"
                              : "Fixed spelling: %s to %s",
                          r->uri, nuri, ref);

            return HTTP_MOVED_PERMANENTLY;
        }
        /*
         * Otherwise, a "[300] Multiple Choices" list with the variants is
         * returned.
