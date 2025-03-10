static proxy_worker *find_best_worker(proxy_balancer *balancer,
                                      request_rec *r)
{
    proxy_worker *candidate = NULL;

    if (PROXY_THREAD_LOCK(balancer) != APR_SUCCESS)
        return NULL;

    candidate = (*balancer->lbmethod->finder)(balancer, r);

/*
        PROXY_THREAD_UNLOCK(balancer);
        return NULL;
*/

    PROXY_THREAD_UNLOCK(balancer);

    if (candidate == NULL) {
        /* All the workers are in error state or disabled.
         * If the balancer has a timeout sleep for a while
         * and try again to find the worker. The chances are
         * that some other thread will release a connection.
         * By default the timeout is not set, and the server
         * returns SERVER_BUSY.
         */
#if APR_HAS_THREADS
        if (balancer->timeout) {
            /* XXX: This can perhaps be build using some
             * smarter mechanism, like tread_cond.
             * But since the statuses can came from
             * different childs, use the provided algo.
             */
            apr_interval_time_t timeout = balancer->timeout;
            apr_interval_time_t step, tval = 0;
            /* Set the timeout to 0 so that we don't
             * end in infinite loop
             */
            balancer->timeout = 0;
            step = timeout / 100;
            while (tval < timeout) {
                apr_sleep(step);
                /* Try again */
                if ((candidate = find_best_worker(balancer, r)))
                    break;
                tval += step;
            }
            /* restore the timeout */
            balancer->timeout = timeout;
        }
#endif
    }
    return candidate;
}