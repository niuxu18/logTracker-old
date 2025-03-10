            conn_rec *c = r->connection;
            ap_log_rerror(APLOG_MARK, APLOG_ERR, status, r, APLOGNO(02609)
                          "read request body failed to %pI (%s)"
                          " from %s (%s)", p_conn->addr,
                          p_conn->hostname ? p_conn->hostname: "",
                          c->client_ip, c->remote_host ? c->remote_host: "");
            return HTTP_BAD_REQUEST;
        }
    }

    if (bytes_streamed != cl_val) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01087)
                      "client %s given Content-Length did not match"
