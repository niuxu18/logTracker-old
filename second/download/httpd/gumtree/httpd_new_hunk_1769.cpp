
            if (SSL_get_state(ssl) != SSL_ST_OK) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                              "Re-negotiation handshake failed: "
                              "Not accepted by client!?");

                r->connection->keepalive = AP_CONN_CLOSE;
                return HTTP_FORBIDDEN;
            }
        }

        /*
         * Remember the peer certificate's DN
