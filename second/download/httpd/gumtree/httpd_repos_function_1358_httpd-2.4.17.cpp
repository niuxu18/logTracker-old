static void socache_shmcb_status(ap_socache_instance_t *ctx,
                                 request_rec *r, int flags)
{
    server_rec *s = r->server;
    SHMCBHeader *header = ctx->header;
    unsigned int loop, total = 0, cache_total = 0, non_empty_subcaches = 0;
    apr_time_t idx_expiry, min_expiry = 0, max_expiry = 0;
    apr_time_t now = apr_time_now();
    double expiry_total = 0;
    int index_pct, cache_pct;

    AP_DEBUG_ASSERT(header->subcache_num > 0);
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(00840) "inside shmcb_status");
    /* Perform the iteration inside the mutex to avoid corruption or invalid
     * pointer arithmetic. The rest of our logic uses read-only header data so
     * doesn't need the lock. */
    /* Iterate over the subcaches */
    for (loop = 0; loop < header->subcache_num; loop++) {
        SHMCBSubcache *subcache = SHMCB_SUBCACHE(header, loop);
        shmcb_subcache_expire(s, header, subcache, now);
        total += subcache->idx_used;
        cache_total += subcache->data_used;
        if (subcache->idx_used) {
            SHMCBIndex *idx = SHMCB_INDEX(subcache, subcache->idx_pos);
            non_empty_subcaches++;
            idx_expiry = idx->expires;
            expiry_total += (double)idx_expiry;
            max_expiry = ((idx_expiry > max_expiry) ? idx_expiry : max_expiry);
            if (!min_expiry)
                min_expiry = idx_expiry;
            else
                min_expiry = ((idx_expiry < min_expiry) ? idx_expiry : min_expiry);
        }
    }
    index_pct = (100 * total) / (header->index_num *
                                 header->subcache_num);
    cache_pct = (100 * cache_total) / (header->subcache_data_size *
                                       header->subcache_num);
    /* Generate Output */
    if (!(flags & AP_STATUS_SHORT)) {
        ap_rprintf(r, "cache type: <b>SHMCB</b>, shared memory: <b>%" APR_SIZE_T_FMT "</b> "
                   "bytes, current entries: <b>%d</b><br>",
                   ctx->shm_size, total);
        ap_rprintf(r, "subcaches: <b>%d</b>, indexes per subcache: <b>%d</b><br>",
                   header->subcache_num, header->index_num);
        if (non_empty_subcaches) {
            apr_time_t average_expiry = (apr_time_t)(expiry_total / (double)non_empty_subcaches);
            ap_rprintf(r, "time left on oldest entries' objects: ");
            if (now < average_expiry)
                ap_rprintf(r, "avg: <b>%d</b> seconds, (range: %d...%d)<br>",
                           (int)apr_time_sec(average_expiry - now),
                           (int)apr_time_sec(min_expiry - now),
                           (int)apr_time_sec(max_expiry - now));
            else
                ap_rprintf(r, "expiry_threshold: <b>Calculation error!</b><br>");
        }

        ap_rprintf(r, "index usage: <b>%d%%</b>, cache usage: <b>%d%%</b><br>",
                   index_pct, cache_pct);
        ap_rprintf(r, "total entries stored since starting: <b>%lu</b><br>",
                   header->stat_stores);
        ap_rprintf(r, "total entries replaced since starting: <b>%lu</b><br>",
                   header->stat_replaced);
        ap_rprintf(r, "total entries expired since starting: <b>%lu</b><br>",
                   header->stat_expiries);
        ap_rprintf(r, "total (pre-expiry) entries scrolled out of the cache: "
                   "<b>%lu</b><br>", header->stat_scrolled);
        ap_rprintf(r, "total retrieves since starting: <b>%lu</b> hit, "
                   "<b>%lu</b> miss<br>", header->stat_retrieves_hit,
                   header->stat_retrieves_miss);
        ap_rprintf(r, "total removes since starting: <b>%lu</b> hit, "
                   "<b>%lu</b> miss<br>", header->stat_removes_hit,
                   header->stat_removes_miss);
    }
    else {
        ap_rputs("CacheType: SHMCB\n", r);
        ap_rprintf(r, "CacheSharedMemory: %" APR_SIZE_T_FMT "\n",
                   ctx->shm_size);
        ap_rprintf(r, "CacheCurrentEntries: %d\n", total);
        ap_rprintf(r, "CacheSubcaches: %d\n", header->subcache_num);
        ap_rprintf(r, "CacheIndexesPerSubcaches: %d\n", header->index_num);
        if (non_empty_subcaches) {
            apr_time_t average_expiry = (apr_time_t)(expiry_total / (double)non_empty_subcaches);
            if (now < average_expiry) {
                ap_rprintf(r, "CacheTimeLeftOldestAvg: %d\n", (int)apr_time_sec(average_expiry - now));
                ap_rprintf(r, "CacheTimeLeftOldestMin: %d\n", (int)apr_time_sec(min_expiry - now));
                ap_rprintf(r, "CacheTimeLeftOldestMax: %d\n", (int)apr_time_sec(max_expiry - now));
            }
        }

        ap_rprintf(r, "CacheIndexUsage: %d%%\n", index_pct);
        ap_rprintf(r, "CacheUsage: %d%%\n", cache_pct);
        ap_rprintf(r, "CacheStoreCount: %lu\n", header->stat_stores);
        ap_rprintf(r, "CacheReplaceCount: %lu\n", header->stat_replaced);
        ap_rprintf(r, "CacheExpireCount: %lu\n", header->stat_expiries);
        ap_rprintf(r, "CacheDiscardCount: %lu\n", header->stat_scrolled);
        ap_rprintf(r, "CacheRetrieveHitCount: %lu\n", header->stat_retrieves_hit);
        ap_rprintf(r, "CacheRetrieveMissCount: %lu\n", header->stat_retrieves_miss);
        ap_rprintf(r, "CacheRemoveHitCount: %lu\n", header->stat_removes_hit);
        ap_rprintf(r, "CacheRemoveMissCount: %lu\n", header->stat_removes_miss);
    }
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(00841) "leaving shmcb_status");
}