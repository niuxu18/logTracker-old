static int read_request_line(request_rec *r, apr_bucket_brigade *bb)
{
    const char *ll;
    const char *uri;
    const char *pro;

#if 0
    conn_rec *conn = r->connection;
#endif
    int major = 1, minor = 0;   /* Assume HTTP/1.0 if non-"HTTP" protocol */
    char http[5];
    apr_size_t len;
    int num_blank_lines = 0;
    int max_blank_lines = r->server->limit_req_fields;

    if (max_blank_lines <= 0) {
        max_blank_lines = DEFAULT_LIMIT_REQUEST_FIELDS;
    }

    /* Read past empty lines until we get a real request line,
     * a read error, the connection closes (EOF), or we timeout.
     *
     * We skip empty lines because browsers have to tack a CRLF on to the end
     * of POSTs to support old CERN webservers.  But note that we may not
     * have flushed any previous response completely to the client yet.
     * We delay the flush as long as possible so that we can improve
     * performance for clients that are pipelining requests.  If a request
     * is pipelined then we won't block during the (implicit) read() below.
     * If the requests aren't pipelined, then the client is still waiting
     * for the final buffer flush from us, and we will block in the implicit
     * read().  B_SAFEREAD ensures that the BUFF layer flushes if it will
     * have to block during a read.
     */

    do {
        apr_status_t rv;

        /* insure ap_rgetline allocates memory each time thru the loop
         * if there are empty lines
         */
        r->the_request = NULL;
        rv = ap_rgetline(&(r->the_request), (apr_size_t)(r->server->limit_req_line + 2),
                         &len, r, 0, bb);

        if (rv != APR_SUCCESS) {
            r->request_time = apr_time_now();

            /* ap_rgetline returns APR_ENOSPC if it fills up the
             * buffer before finding the end-of-line.  This is only going to
             * happen if it exceeds the configured limit for a request-line.
             */
            if (rv == APR_ENOSPC) {
                r->status    = HTTP_REQUEST_URI_TOO_LARGE;
                r->proto_num = HTTP_VERSION(1,0);
                r->protocol  = apr_pstrdup(r->pool, "HTTP/1.0");
            }
            else if (APR_STATUS_IS_TIMEUP(rv)) {
                r->status = HTTP_REQUEST_TIME_OUT;
            }
            return 0;
        }
    } while ((len <= 0) && (++num_blank_lines < max_blank_lines));

    /* we've probably got something to do, ignore graceful restart requests */

    r->request_time = apr_time_now();
    ll = r->the_request;
    r->method = ap_getword_white(r->pool, &ll);

#if 0
/* XXX If we want to keep track of the Method, the protocol module should do
 * it.  That support isn't in the scoreboard yet.  Hopefully next week
 * sometime.   rbb */
    ap_update_connection_status(AP_CHILD_THREAD_FROM_ID(conn->id), "Method",
                                r->method);
#endif

    uri = ap_getword_white(r->pool, &ll);

    /* Provide quick information about the request method as soon as known */

    r->method_number = ap_method_number_of(r->method);
    if (r->method_number == M_GET && r->method[0] == 'H') {
        r->header_only = 1;
    }

    ap_parse_uri(r, uri);

    if (ll[0]) {
        r->assbackwards = 0;
        pro = ll;
        len = strlen(ll);
    } else {
        r->assbackwards = 1;
        pro = "HTTP/0.9";
        len = 8;
    }
    r->protocol = apr_pstrmemdup(r->pool, pro, len);

    /* XXX ap_update_connection_status(conn->id, "Protocol", r->protocol); */

    /* Avoid sscanf in the common case */
    if (len == 8
        && pro[0] == 'H' && pro[1] == 'T' && pro[2] == 'T' && pro[3] == 'P'
        && pro[4] == '/' && apr_isdigit(pro[5]) && pro[6] == '.'
        && apr_isdigit(pro[7])) {
        r->proto_num = HTTP_VERSION(pro[5] - '0', pro[7] - '0');
    }
    else if (3 == sscanf(r->protocol, "%4s/%u.%u", http, &major, &minor)
             && (strcasecmp("http", http) == 0)
             && (minor < HTTP_VERSION(1, 0)) ) /* don't allow HTTP/0.1000 */
        r->proto_num = HTTP_VERSION(major, minor);
    else
        r->proto_num = HTTP_VERSION(1, 0);

    return 1;
}