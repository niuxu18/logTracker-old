static void start_connect(struct connection * c)
{
    apr_status_t rv;

    if (!(started < requests))
    return;

    c->read = 0;
    c->bread = 0;
    c->keepalive = 0;
    c->cbx = 0;
    c->gotheader = 0;
    c->rwrite = 0;
    if (c->ctx)
        apr_pool_destroy(c->ctx);
    apr_pool_create(&c->ctx, cntxt);

    if ((rv = apr_socket_create(&c->aprsock, destsa->family,
                SOCK_STREAM, 0, c->ctx)) != APR_SUCCESS) {
    apr_err("socket", rv);
    }
    if ((rv = apr_socket_opt_set(c->aprsock, APR_SO_NONBLOCK, 1))
         != APR_SUCCESS) {
        apr_err("socket nonblock", rv);
    }
    c->start = apr_time_now();
#ifdef USE_SSL
    if (is_ssl) {
        BIO *bio;
        apr_os_sock_t fd;

        if ((c->ssl = SSL_new(ssl_ctx)) == NULL) {
            BIO_printf(bio_err, "SSL_new failed.\n");
            ERR_print_errors(bio_err);
            exit(1);
        }
        ssl_rand_seed();
        apr_os_sock_get(&fd, c->aprsock);
        bio = BIO_new_socket(fd, BIO_NOCLOSE);
        SSL_set_bio(c->ssl, bio, bio);
        SSL_set_connect_state(c->ssl);
        if (verbosity >= 4) {
            BIO_set_callback(bio, ssl_print_cb);
            BIO_set_callback_arg(bio, (void *)bio_err);
        }
    } else {
        c->ssl = NULL;
    }
#endif
    if ((rv = apr_socket_connect(c->aprsock, destsa)) != APR_SUCCESS) {
        if (APR_STATUS_IS_EINPROGRESS(rv)) {
            apr_pollfd_t new_pollfd;
            c->state = STATE_CONNECTING;
            c->rwrite = 0;
            new_pollfd.desc_type = APR_POLL_SOCKET;
            new_pollfd.reqevents = APR_POLLOUT;
            new_pollfd.desc.s = c->aprsock;
            new_pollfd.client_data = c;
            apr_pollset_add(readbits, &new_pollfd);
            return;
        }
        else {
            apr_pollfd_t remove_pollfd;
            remove_pollfd.desc_type = APR_POLL_SOCKET;
            remove_pollfd.desc.s = c->aprsock;
            apr_pollset_remove(readbits, &remove_pollfd);
            apr_socket_close(c->aprsock);
            err_conn++;
            if (bad++ > 10) {
                fprintf(stderr,
                   "\nTest aborted after 10 failures\n\n");
                apr_err("apr_socket_connect()", rv);
            }
            c->state = STATE_UNCONNECTED;
            start_connect(c);
            return;
        }
    }

    /* connected first time */
    c->state = STATE_CONNECTED;
    started++;
#ifdef USE_SSL
    if (c->ssl) {
        ssl_proceed_handshake(c);
    } else
#endif
    {
        write_request(c);
    }
}