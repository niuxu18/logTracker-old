
        if (rc != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_ERR, rc, ap_server_conf,
                         "process_socket: apr_pollset_add failure");
            AP_DEBUG_ASSERT(rc == APR_SUCCESS);
        }
        return;
    }
    else if (cs->pub.state == CONN_STATE_SUSPENDED) {
        apr_atomic_inc32(&suspended_count);
    }
    /*
     * Prevent this connection from writing to our connection state after it
     * is no longer associated with this thread. This would happen if the EOR
     * bucket is destroyed from the listener thread due to a connection abort
     * or timeout.
     */
    c->sbh = NULL;
    return;
}

/* conns_this_child has gone to zero or below.  See if the admin coded
   "MaxConnectionsPerChild 0", and keep going in that case.  Doing it this way
   simplifies the hot path in worker_thread */
static void check_infinite_requests(void)
