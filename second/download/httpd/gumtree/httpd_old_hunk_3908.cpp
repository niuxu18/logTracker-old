             * we have to send out the response to this one in h2
             * format. So we need to take over the connection
             * right away.
             */
            ap_remove_input_filter_byhandle(r->input_filters, "http_in");
            ap_remove_input_filter_byhandle(r->input_filters, "reqtimeout");
            
            /* Ok, start an h2_conn on this one. */
            status = h2_conn_rprocess(r);
            if (status != DONE) {
                /* Nothing really to do about this. */
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, status, r,
                              "session proessed, unexpected status");
            }
        }
