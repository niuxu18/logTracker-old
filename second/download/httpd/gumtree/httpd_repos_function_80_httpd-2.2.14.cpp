static void write_request(struct connection * c)
{
    do {
        apr_time_t tnow;
        apr_size_t l = c->rwrite;
        apr_status_t e = APR_SUCCESS; /* prevent gcc warning */

        tnow = lasttime = apr_time_now();

        /*
         * First time round ?
         */
        if (c->rwrite == 0) {
            apr_socket_timeout_set(c->aprsock, 0);
            c->connect = tnow;
            c->rwrote = 0;
            c->rwrite = reqlen;
            if (posting)
                c->rwrite += postlen;
        }
        else if (tnow > c->connect + aprtimeout) {
            printf("Send request timed out!\n");
            close_connection(c);
            return;
        }

#ifdef USE_SSL
        if (c->ssl) {
            apr_size_t e_ssl;
            e_ssl = SSL_write(c->ssl,request + c->rwrote, l);
            if (e_ssl != l) {
                BIO_printf(bio_err, "SSL write failed - closing connection\n");
                ERR_print_errors(bio_err);
                close_connection (c);
                return;
            }
            l = e_ssl;
            e = APR_SUCCESS;
        }
        else
#endif
            e = apr_socket_send(c->aprsock, request + c->rwrote, &l);

        if (e != APR_SUCCESS && !APR_STATUS_IS_EAGAIN(e)) {
            epipe++;
            printf("Send request failed!\n");
            close_connection(c);
            return;
        }
        totalposted += l;
        c->rwrote += l;
        c->rwrite -= l;
    } while (c->rwrite);

    c->state = STATE_READ;
    c->endwrite = lasttime = apr_time_now();
    {
        apr_pollfd_t new_pollfd;
        new_pollfd.desc_type = APR_POLL_SOCKET;
        new_pollfd.reqevents = APR_POLLIN;
        new_pollfd.desc.s = c->aprsock;
        new_pollfd.client_data = c;
        apr_pollset_add(readbits, &new_pollfd);
    }
}