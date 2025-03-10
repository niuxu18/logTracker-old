     *
     * Handle two way transfer of data over the socket (this is a tunnel).
     */

/*    r->sent_bodyct = 1;*/

    if ((rv = apr_pollset_create(&pollset, 2, r->pool, 0)) != APR_SUCCESS)
    {
    apr_socket_close(sock);
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
            "proxy: CONNECT: error apr_pollset_create()");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    /* Add client side to the poll */
