static proxy_worker *find_route_worker(proxy_balancer *balancer,
                                       const char *route, request_rec *r,
                                       int recursion)
{
    int i;
    int checking_standby;
    int checked_standby;

    proxy_worker **workers;

    checking_standby = checked_standby = 0;
    while (!checked_standby) {
        workers = (proxy_worker **)balancer->workers->elts;
        for (i = 0; i < balancer->workers->nelts; i++, workers++) {
            proxy_worker *worker = *workers;
            if ( (checking_standby ? !PROXY_WORKER_IS_STANDBY(worker) : PROXY_WORKER_IS_STANDBY(worker)) )
                continue;
            if (*(worker->s->route) && strcmp(worker->s->route, route) == 0) {
                if (PROXY_WORKER_IS_USABLE(worker)) {
                    return worker;
                } else {
                    /*
                     * If the worker is in error state run
                     * retry on that worker. It will be marked as
                     * operational if the retry timeout is elapsed.
                     * The worker might still be unusable, but we try
                     * anyway.
                     */
                    ap_proxy_retry_worker_fn("BALANCER", worker, r->server);
                    if (PROXY_WORKER_IS_USABLE(worker)) {
                            return worker;
                    } else {
                        /*
                         * We have a worker that is unusable.
                         * It can be in error or disabled, but in case
                         * it has a redirection set use that redirection worker.
                         * This enables to safely remove the member from the
                         * balancer. Of course you will need some kind of
                         * session replication between those two remote.
                         * Also check that we haven't gone thru all the
                         * balancer members by means of redirects.
                         * This should avoid redirect cycles.
                         */
                        if ((*worker->s->redirect)
                            && (recursion < balancer->workers->nelts)) {
                            proxy_worker *rworker = NULL;
                            rworker = find_route_worker(balancer, worker->s->redirect,
                                                        r, recursion + 1);
                            /* Check if the redirect worker is usable */
                            if (rworker && !PROXY_WORKER_IS_USABLE(rworker)) {
                                /*
                                 * If the worker is in error state run
                                 * retry on that worker. It will be marked as
                                 * operational if the retry timeout is elapsed.
                                 * The worker might still be unusable, but we try
                                 * anyway.
                                 */
                                ap_proxy_retry_worker_fn("BALANCER", rworker, r->server);
                            }
                            if (rworker && PROXY_WORKER_IS_USABLE(rworker))
                                return rworker;
                        }
                    }
                }
            }
        }
        checked_standby = checking_standby++;
    }
    return NULL;
}