#if APR_FILES_AS_SOCKETS
                int num_events;
                const apr_pollfd_t *pdesc;

                rv = apr_pollset_poll(ctx->pollset, f->r->server->timeout,
                                      &num_events, &pdesc);
                if (rv || dc->debug >= DBGLVL_GORY) {
                    ap_log_rerror(APLOG_MARK, APLOG_DEBUG,
                                  rv, f->r, "apr_pollset_poll()");
                }
                if (rv != APR_SUCCESS && !APR_STATUS_IS_EINTR(rv)) {
                    /* some error such as APR_TIMEUP */
                    return rv;
                }
#else /* APR_FILES_AS_SOCKETS */
                /* Yuck... I'd really like to wait until I can read
                 * or write, but instead I have to sleep and try again
                 */
                apr_sleep(100000); /* 100 milliseconds */
                if (dc->debug >= DBGLVL_GORY) {
                    ap_log_rerror(APLOG_MARK, APLOG_DEBUG,
                                  0, f->r, "apr_sleep()");
                }
#endif /* APR_FILES_AS_SOCKETS */
            }
            else if (rv != APR_SUCCESS) {
                return rv;
            }
        }
