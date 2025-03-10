static void purge(char *path, apr_pool_t *pool, apr_off_t max)
{
    ENTRY *e, *n, *oldest;

    struct stats s;
    s.sum = 0;
    s.entries = 0;
    s.dfuture = 0;
    s.dexpired = 0;
    s.dfresh = 0;
    s.max = max;

    for (e = APR_RING_FIRST(&root);
         e != APR_RING_SENTINEL(&root, _entry, link);
         e = APR_RING_NEXT(e, link)) {
        s.sum += e->hsize;
        s.sum += e->dsize;
        s.entries++;
    }

    s.total = s.sum;
    s.etotal = s.entries;

    if (s.sum <= s.max) {
        printstats(path, &s);
        return;
    }

    /* process all entries with a timestamp in the future, this may
     * happen if a wrong system time is corrected
     */

    for (e = APR_RING_FIRST(&root);
         e != APR_RING_SENTINEL(&root, _entry, link) && !interrupted;) {
        n = APR_RING_NEXT(e, link);
        if (e->response_time > now || e->htime > now || e->dtime > now) {
            delete_entry(path, e->basename, pool);
            s.sum -= e->hsize;
            s.sum -= e->dsize;
            s.entries--;
            s.dfuture++;
            APR_RING_REMOVE(e, link);
            if (s.sum <= s.max) {
                if (!interrupted) {
                    printstats(path, &s);
                }
                return;
            }
        }
        e = n;
    }

    if (interrupted) {
        return;
    }

    /* process all entries with are expired */
    for (e = APR_RING_FIRST(&root);
         e != APR_RING_SENTINEL(&root, _entry, link) && !interrupted;) {
        n = APR_RING_NEXT(e, link);
        if (e->expire != APR_DATE_BAD && e->expire < now) {
            delete_entry(path, e->basename, pool);
            s.sum -= e->hsize;
            s.sum -= e->dsize;
            s.entries--;
            s.dexpired++;
            APR_RING_REMOVE(e, link);
            if (s.sum <= s.max) {
                if (!interrupted) {
                    printstats(path, &s);
                }
                return;
            }
        }
        e = n;
    }

    if (interrupted) {
         return;
    }

    /* process remaining entries oldest to newest, the check for an emtpy
     * ring actually isn't necessary except when the compiler does
     * corrupt 64bit arithmetics which happend to me once, so better safe
     * than sorry
     */
    while (s.sum > s.max && !interrupted
           && !APR_RING_EMPTY(&root, _entry, link)) {
        oldest = APR_RING_FIRST(&root);

        for (e = APR_RING_NEXT(oldest, link);
             e != APR_RING_SENTINEL(&root, _entry, link);
             e = APR_RING_NEXT(e, link)) {
            if (e->dtime < oldest->dtime) {
                oldest = e;
            }
        }

        delete_entry(path, oldest->basename, pool);
        s.sum -= oldest->hsize;
        s.sum -= oldest->dsize;
        s.entries--;
        s.dfresh++;
        APR_RING_REMOVE(oldest, link);
    }

    if (!interrupted) {
        printstats(path, &s);
    }
}