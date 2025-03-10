}
static int authn_cache_post_config(apr_pool_t *pconf, apr_pool_t *plog,
                                   apr_pool_t *ptmp, server_rec *s)
{
    apr_status_t rv;
    static struct ap_socache_hints authn_cache_hints = {64, 32, 60000000};

    if (!configured) {
        return OK;    /* don't waste the overhead of creating mutex & cache */
    }
    if (socache_provider == NULL) {
        ap_log_perror(APLOG_MARK, APLOG_CRIT, 0, plog, APLOGNO(01674)
                      "Please select a socache provider with AuthnCacheSOCache "
                      "(no default found on this platform). Maybe you need to "
                      "load mod_socache_shmcb or another socache module first");
        return 500; /* An HTTP status would be a misnomer! */
    }

    rv = ap_global_mutex_create(&authn_cache_mutex, NULL,
                                authn_cache_id, NULL, s, pconf, 0);
    if (rv != APR_SUCCESS) {
        ap_log_perror(APLOG_MARK, APLOG_CRIT, rv, plog, APLOGNO(01675)
                      "failed to create %s mutex", authn_cache_id);
        return 500; /* An HTTP status would be a misnomer! */
