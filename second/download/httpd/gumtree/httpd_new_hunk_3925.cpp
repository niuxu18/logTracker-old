            /* log the situation */
            ap_log_cerror(APLOG_MARK, APLOG_INFO, 0, f->c, APLOGNO(01996)
                         "SSL handshake failed: HTTP spoken on HTTPS port; "
                         "trying to send HTML error page");
            ssl_log_ssl_error(SSLLOG_MARK, APLOG_INFO, sslconn->server);

            ssl_io_filter_disable(sslconn, f);
            f->c->keepalive = AP_CONN_CLOSE;
            if (is_init) {
                sslconn->non_ssl_request = NON_SSL_SEND_REQLINE;
                return APR_EGENERAL;
            }
            sslconn->non_ssl_request = NON_SSL_SEND_HDR_SEP;

            /* fake the request line */
            bucket = HTTP_ON_HTTPS_PORT_BUCKET(f->c->bucket_alloc);
            send_eos = 0;
            break;

