        ap_log_error(APLOG_MARK, log_level, rv, ap_server_conf, APLOGNO(00056)
                     "connect to listener on %pI", lp->bind_addr);
        apr_pool_destroy(p);
        return rv;
    }

    if (lp->protocol && strcasecmp(lp->protocol, "https") == 0) {
        /* Send a TLS 1.0 close_notify alert.  This is perhaps the
         * "least wrong" way to open and cleanly terminate an SSL
         * connection.  It should "work" without noisy error logs if
         * the server actually expects SSLv3/TLSv1.  With
         * SSLv23_server_method() OpenSSL's SSL_accept() fails
         * ungracefully on receipt of this message, since it requires
         * an 11-byte ClientHello message and this is too short. */
        static const unsigned char tls10_close_notify[7] = {
            '\x15',         /* TLSPlainText.type = Alert (21) */
            '\x03', '\x01', /* TLSPlainText.version = {3, 1} */
            '\x00', '\x02', /* TLSPlainText.length = 2 */
            '\x01',         /* Alert.level = warning (1) */
            '\x00'          /* Alert.description = close_notify (0) */
        };
        data = (const char *)tls10_close_notify;
        len = sizeof(tls10_close_notify);
    }
    else /* ... XXX other request types here? */ {
        /* Create an HTTP request string.  We include a User-Agent so
         * that adminstrators can track down the cause of the
         * odd-looking requests in their logs.  A complete request is
         * used since kernel-level filtering may require that much
         * data before returning from accept(). */
        data = apr_pstrcat(p, "OPTIONS * HTTP/1.0\r\nUser-Agent: ",
                           ap_get_server_description(),
                           " (internal dummy connection)\r\n\r\n", NULL);
        len = strlen(data);
    }

    apr_socket_send(sock, data, &len);
    apr_socket_close(sock);
    apr_pool_destroy(p);

    return rv;
}

