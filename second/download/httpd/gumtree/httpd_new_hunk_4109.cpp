                pollevent = cur->rtnevents;
                if (pollevent & (APR_POLLIN | APR_POLLHUP)) {
#ifdef DEBUGGING
                    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01027)
                                  "client was readable");
#endif
                    done |= ap_proxy_transfer_between_connections(r, c,
                                                                  backconn,
                                                                  bb_front,
                                                                  bb_back,
                                                                  "client",
                                                                  NULL,
                                                                  CONN_BLKSZ, 1)
                                                                 != APR_SUCCESS;
                }
                else if (pollevent & APR_POLLERR) {
                    ap_log_rerror(APLOG_MARK, APLOG_NOTICE, 0, r, APLOGNO(02827)
                                  "err on client");
                    c->aborted = 1;
                    done = 1;
