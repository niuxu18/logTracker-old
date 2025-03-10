void ap_push_pool(fd_queue_info_t * queue_info,
                                    apr_pool_t * pool_to_recycle)
{
    /* If we have been given a pool to recycle, atomically link
     * it into the queue_info's list of recycled pools
     */
    if (pool_to_recycle) {
        struct recycled_pool *new_recycle;
        new_recycle = (struct recycled_pool *) apr_palloc(pool_to_recycle,
                                                          sizeof
                                                          (*new_recycle));
        new_recycle->pool = pool_to_recycle;
        for (;;) {
            /*
             * Save queue_info->recycled_pool in local variable next because
             * new_recycle->next can be changed after apr_atomic_casptr
             * function call. For gory details see PR 44402.
             */
            struct recycled_pool *next = queue_info->recycled_pools;
            new_recycle->next = next;
            if (apr_atomic_casptr
                ((volatile void **) &(queue_info->recycled_pools),
                 new_recycle, next) == next) {
                break;
            }
        }
    }
}