                            ap_log_cerror( APLOG_MARK, APLOG_TRACE3, status, c,
                                          "h2_session(%ld): keepalive, %f sec left",
                                          session->id, (session->idle_until - now) / 1000000.0f);
                        }
                        /* continue reading handling */
                    }
                    else if (APR_STATUS_IS_ECONNABORTED(status)
                             || APR_STATUS_IS_ECONNRESET(status)
                             || APR_STATUS_IS_EOF(status)
                             || APR_STATUS_IS_EBADF(status)) {
                        ap_log_cerror( APLOG_MARK, APLOG_TRACE3, status, c,
                                      "h2_session(%ld): input gone", session->id);
                        dispatch_event(session, H2_SESSION_EV_CONN_ERROR, 0, NULL);
                    }
                    else {
                        ap_log_cerror( APLOG_MARK, APLOG_TRACE3, status, c,
                                      "h2_session(%ld): idle(1 sec timeout) "
                                      "read failed", session->id);
                        dispatch_event(session, H2_SESSION_EV_CONN_ERROR, 0, "error");
                    }
                }
                
                break;
                
            case H2_SESSION_ST_BUSY:
                if (nghttp2_session_want_read(session->ngh2)) {
                    ap_update_child_status(session->c->sbh, SERVER_BUSY_READ, NULL);
                    h2_filter_cin_timeout_set(session->cin, session->s->timeout);
                    status = h2_session_read(session, 0);
                    if (status == APR_SUCCESS) {
                        session->have_read = 1;
