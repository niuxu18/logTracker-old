    }

    /* Always return the SUCCESS */
    return APR_SUCCESS;
}

PROXY_DECLARE(apr_status_t) ap_proxy_ssl_connection_cleanup(proxy_conn_rec *conn,
                                                            request_rec *r)
{
    apr_bucket_brigade *bb;
    apr_status_t rv;

    /*
     * If we have an existing SSL connection it might be possible that the
     * server sent some SSL message we have not read so far (e.g. an SSL
     * shutdown message if the server closed the keepalive connection while
     * the connection was held unused in our pool).
     * So ensure that if present (=> APR_NONBLOCK_READ) it is read and
     * processed. We don't expect any data to be in the returned brigade.
     */
    if (conn->sock && conn->connection) {
        bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
        rv = ap_get_brigade(conn->connection->input_filters, bb,
                            AP_MODE_READBYTES, APR_NONBLOCK_READ,
                            HUGE_STRING_LEN);
        if ((rv != APR_SUCCESS) && !APR_STATUS_IS_EAGAIN(rv)) {
            socket_cleanup(conn);
        }
        if (!APR_BRIGADE_EMPTY(bb)) {
            apr_off_t len;

            rv = apr_brigade_length(bb, 0, &len);
            ap_log_rerror(APLOG_MARK, APLOG_TRACE3, rv, r,
                          "SSL cleanup brigade contained %"
                          APR_OFF_T_FMT " bytes of data.", len);
        }
        apr_brigade_destroy(bb);
    }
    return APR_SUCCESS;
}

/* reslist constructor */
static apr_status_t connection_constructor(void **resource, void *params,
