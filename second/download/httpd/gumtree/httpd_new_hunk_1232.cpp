             * seamonkey only display an empty page in this case and do
             * not do a retry. We should also not do this on a
             * connection which times out; instead handle as
             * we normally would handle timeouts
             */
            if (r->proxyreq == PROXYREQ_REVERSE && c->keepalives &&
                !APR_STATUS_IS_TIMEUP(rc)) {
                apr_bucket *eos;

                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r,
                              "proxy: Closing connection to client because"
                              " reading from backend server %s:%d failed."
                              " Number of keepalives %i", backend->hostname, 
                              backend->port, c->keepalives);
                ap_proxy_backend_broke(r, bb);
                /*
                 * Add an EOC bucket to signal the ap_http_header_filter
                 * that it should get out of our way, BUT ensure that the
                 * EOC bucket is inserted BEFORE an EOS bucket in bb as
                 * some resource filters like mod_deflate pass everything
