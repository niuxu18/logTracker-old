    /* let's have our own pool that will be parent to all h2_worker
     * instances we create. This happens in various threads, but always
     * guarded by our lock. Without this pool, all subpool creations would
     * happen on the pool handed to us, which we do not guard.
     */
    apr_pool_create(&pool, server_pool);
    workers = apr_pcalloc(pool, sizeof(h2_workers));
    if (workers) {
        workers->s = s;
        workers->pool = pool;
        workers->min_size = min_size;
        workers->max_size = max_size;
        apr_atomic_set32(&workers->max_idle_secs, 10);
        
        apr_threadattr_create(&workers->thread_attr, workers->pool);
        
        APR_RING_INIT(&workers->workers, h2_worker, link);
        APR_RING_INIT(&workers->zombies, h2_worker, link);
        APR_RING_INIT(&workers->mplxs, h2_mplx, link);
        
        status = apr_thread_mutex_create(&workers->lock,
