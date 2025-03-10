                }
            }
        }

        connected    = 1;
    }
    /*
     * Put the entire worker to error state if
     * the PROXY_WORKER_IGNORE_ERRORS flag is not set.
     * Altrough some connections may be alive
     * no further connections to the worker could be made
     */
    if (!connected && PROXY_WORKER_IS_USABLE(worker) &&
        !(worker->s->status & PROXY_WORKER_IGNORE_ERRORS)) {
        worker->s->status |= PROXY_WORKER_IN_ERROR;
        worker->s->error_time = apr_time_now();
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
            "ap_proxy_connect_backend disabling worker for (%s)",
            worker->hostname);
    }
    else {
        if (worker->s->retries) {
            /*
             * A worker came back. So here is where we need to
             * either reset all params to initial conditions or
             * apply some sort of aging
             */
        }
        worker->s->error_time = 0;
        worker->s->retries = 0;
    }
    return connected ? OK : DECLINED;
}

PROXY_DECLARE(int) ap_proxy_connection_create(const char *proxy_function,
                                              proxy_conn_rec *conn,
                                              conn_rec *c,
                                              server_rec *s)
