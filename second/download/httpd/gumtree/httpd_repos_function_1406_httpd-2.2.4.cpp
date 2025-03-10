static int proxy_connect_handler(request_rec *r, proxy_worker *worker,
                                 proxy_server_conf *conf,
                                 char *url, const char *proxyname,
                                 apr_port_t proxyport)
{
    apr_pool_t *p = r->pool;
    apr_socket_t *sock;
    apr_status_t err, rv;
    apr_size_t i, o, nbytes;
    char buffer[HUGE_STRING_LEN];
    apr_socket_t *client_socket = ap_get_module_config(r->connection->conn_config, &core_module);
    int failed;
    apr_pollset_t *pollset;
    apr_pollfd_t pollfd;
    const apr_pollfd_t *signalled;
    apr_int32_t pollcnt, pi;
    apr_int16_t pollevent;
    apr_sockaddr_t *uri_addr, *connect_addr;

    apr_uri_t uri;
    const char *connectname;
    int connectport = 0;

    /* is this for us? */
    if (r->method_number != M_CONNECT) {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
             "proxy: CONNECT: declining URL %s", url);
    return DECLINED;
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
         "proxy: CONNECT: serving URL %s", url);


    /*
     * Step One: Determine Who To Connect To
     *
     * Break up the URL to determine the host to connect to
     */

    /* we break the URL into host, port, uri */
    if (APR_SUCCESS != apr_uri_parse_hostinfo(p, url, &uri)) {
    return ap_proxyerror(r, HTTP_BAD_REQUEST,
                 apr_pstrcat(p, "URI cannot be parsed: ", url, NULL));
    }

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
         "proxy: CONNECT: connecting %s to %s:%d", url, uri.hostname, uri.port);

    /* do a DNS lookup for the destination host */
    err = apr_sockaddr_info_get(&uri_addr, uri.hostname, APR_UNSPEC, uri.port, 0, p);

    /* are we connecting directly, or via a proxy? */
    if (proxyname) {
    connectname = proxyname;
    connectport = proxyport;
        err = apr_sockaddr_info_get(&connect_addr, proxyname, APR_UNSPEC, proxyport, 0, p);
    }
    else {
    connectname = uri.hostname;
    connectport = uri.port;
    connect_addr = uri_addr;
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
         "proxy: CONNECT: connecting to remote proxy %s on port %d", connectname, connectport);

    /* check if ProxyBlock directive on this host */
    if (OK != ap_proxy_checkproxyblock(r, conf, uri_addr)) {
    return ap_proxyerror(r, HTTP_FORBIDDEN,
                 "Connect to remote machine blocked");
    }

    /* Check if it is an allowed port */
    if (conf->allowed_connect_ports->nelts == 0) {
    /* Default setting if not overridden by AllowCONNECT */
    switch (uri.port) {
        case APR_URI_HTTPS_DEFAULT_PORT:
        case APR_URI_SNEWS_DEFAULT_PORT:
        break;
        default:
                /* XXX can we call ap_proxyerror() here to get a nice log message? */
        return HTTP_FORBIDDEN;
    }
    } else if(!allowed_port(conf, uri.port)) {
        /* XXX can we call ap_proxyerror() here to get a nice log message? */
    return HTTP_FORBIDDEN;
    }

    /*
     * Step Two: Make the Connection
     *
     * We have determined who to connect to. Now make the connection.
     */

    /* get all the possible IP addresses for the destname and loop through them
     * until we get a successful connection
     */
    if (APR_SUCCESS != err) {
    return ap_proxyerror(r, HTTP_BAD_GATEWAY, apr_pstrcat(p,
                             "DNS lookup failure for: ",
                             connectname, NULL));
    }

    /*
     * At this point we have a list of one or more IP addresses of
     * the machine to connect to. If configured, reorder this
     * list so that the "best candidate" is first try. "best
     * candidate" could mean the least loaded server, the fastest
     * responding server, whatever.
     *
     * For now we do nothing, ie we get DNS round robin.
     * XXX FIXME
     */
    failed = ap_proxy_connect_to_backend(&sock, "CONNECT", connect_addr,
                                         connectname, conf, r->server,
                                         r->pool);

    /* handle a permanent error from the above loop */
    if (failed) {
        if (proxyname) {
            return DECLINED;
        }
        else {
            return HTTP_BAD_GATEWAY;
        }
    }

    /*
     * Step Three: Send the Request
     *
     * Send the HTTP/1.1 CONNECT request to the remote server
     */

    /* we are acting as a tunnel - the output filter stack should
     * be completely empty, because when we are done here we are done completely.
     * We add the NULL filter to the stack to do this...
     */
    r->output_filters = NULL;
    r->connection->output_filters = NULL;


    /* If we are connecting through a remote proxy, we need to pass
     * the CONNECT request on to it.
     */
    if (proxyport) {
    /* FIXME: Error checking ignored.
     */
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
             "proxy: CONNECT: sending the CONNECT request to the remote proxy");
        nbytes = apr_snprintf(buffer, sizeof(buffer),
                  "CONNECT %s HTTP/1.0" CRLF, r->uri);
        apr_socket_send(sock, buffer, &nbytes);
        nbytes = apr_snprintf(buffer, sizeof(buffer),
                  "Proxy-agent: %s" CRLF CRLF, ap_get_server_banner());
        apr_socket_send(sock, buffer, &nbytes);
    }
    else {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
             "proxy: CONNECT: Returning 200 OK Status");
        nbytes = apr_snprintf(buffer, sizeof(buffer),
                  "HTTP/1.0 200 Connection Established" CRLF);
        ap_xlate_proto_to_ascii(buffer, nbytes);
        apr_socket_send(client_socket, buffer, &nbytes);
        nbytes = apr_snprintf(buffer, sizeof(buffer),
                  "Proxy-agent: %s" CRLF CRLF, ap_get_server_banner());
        ap_xlate_proto_to_ascii(buffer, nbytes);
        apr_socket_send(client_socket, buffer, &nbytes);
#if 0
        /* This is safer code, but it doesn't work yet.  I'm leaving it
         * here so that I can fix it later.
         */
        r->status = HTTP_OK;
        r->header_only = 1;
        apr_table_set(r->headers_out, "Proxy-agent: %s", ap_get_server_banner());
        ap_rflush(r);
#endif
    }

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
         "proxy: CONNECT: setting up poll()");

    /*
     * Step Four: Handle Data Transfer
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
    pollfd.p = r->pool;
    pollfd.desc_type = APR_POLL_SOCKET;
    pollfd.reqevents = APR_POLLIN;
    pollfd.desc.s = client_socket;
    pollfd.client_data = NULL;
    apr_pollset_add(pollset, &pollfd);

    /* Add the server side to the poll */
    pollfd.desc.s = sock;
    apr_pollset_add(pollset, &pollfd);

    while (1) { /* Infinite loop until error (one side closes the connection) */
        if ((rv = apr_pollset_poll(pollset, -1, &pollcnt, &signalled)) != APR_SUCCESS) {
        apr_socket_close(sock);
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, "proxy: CONNECT: error apr_poll()");
            return HTTP_INTERNAL_SERVER_ERROR;
        }
#ifdef DEBUGGING
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                     "proxy: CONNECT: woke from select(), i=%d", pollcnt);
#endif

        for (pi = 0; pi < pollcnt; pi++) {
            const apr_pollfd_t *cur = &signalled[pi];

            if (cur->desc.s == sock) {
                pollevent = cur->rtnevents;
                if (pollevent & APR_POLLIN) {
#ifdef DEBUGGING
                    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                                 "proxy: CONNECT: sock was set");
#endif
                    nbytes = sizeof(buffer);
                    rv = apr_socket_recv(sock, buffer, &nbytes);
                    if (rv == APR_SUCCESS) {
                        o = 0;
                        i = nbytes;
                        while(i > 0)
                        {
                            nbytes = i;
    /* This is just plain wrong.  No module should ever write directly
     * to the client.  For now, this works, but this is high on my list of
     * things to fix.  The correct line is:
     * if ((nbytes = ap_rwrite(buffer + o, nbytes, r)) < 0)
     * rbb
     */
                            rv = apr_socket_send(client_socket, buffer + o, &nbytes);
                            if (rv != APR_SUCCESS)
                                break;
                            o += nbytes;
                            i -= nbytes;
                        }
                    }
                    else
                        break;
                }
                else if ((pollevent & APR_POLLERR) || (pollevent & APR_POLLHUP))
                    break;
            }
            else if (cur->desc.s == client_socket) {
                pollevent = cur->rtnevents;
                if (pollevent & APR_POLLIN) {
#ifdef DEBUGGING
                    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                                 "proxy: CONNECT: client was set");
#endif
                    nbytes = sizeof(buffer);
                    rv = apr_socket_recv(client_socket, buffer, &nbytes);
                    if (rv == APR_SUCCESS) {
                        o = 0;
                        i = nbytes;
#ifdef DEBUGGING
                        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
                                     "proxy: CONNECT: read %d from client", i);
#endif
                        while(i > 0)
                        {
                            nbytes = i;
                            rv = apr_socket_send(sock, buffer + o, &nbytes);
                            if (rv != APR_SUCCESS)
                                break;
                            o += nbytes;
                            i -= nbytes;
                        }
                    }
                    else
                        break;
                }
                else if ((pollevent & APR_POLLERR) || (pollevent & APR_POLLHUP)) {
                    rv = APR_EOF;
                    break;
                }
            }
            else
                break;
        }
        if (rv != APR_SUCCESS) {
            break;
        }
    }

    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server,
         "proxy: CONNECT: finished with poll() - cleaning up");

    /*
     * Step Five: Clean Up
     *
     * Close the socket and clean up
     */

    apr_socket_close(sock);

    return OK;
}