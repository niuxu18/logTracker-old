static void stapling_get_cached_response(server_rec *s, OCSP_RESPONSE **prsp,
                                         BOOL *pok, certinfo *cinf,
                                         apr_pool_t *pool)
{
    SSLModConfigRec *mc = myModConfig(s);
    apr_status_t rv;
    OCSP_RESPONSE *rsp;
    unsigned char resp_der[MAX_STAPLING_DER];
    const unsigned char *p;
    unsigned int resp_derlen = MAX_STAPLING_DER;

    if (mc->stapling_cache->flags & AP_SOCACHE_FLAG_NOTMPSAFE)
        stapling_cache_mutex_on(s);
    rv = mc->stapling_cache->retrieve(mc->stapling_cache_context, s,
                                      cinf->idx, sizeof(cinf->idx),
                                      resp_der, &resp_derlen, pool);
    if (mc->stapling_cache->flags & AP_SOCACHE_FLAG_NOTMPSAFE)
        stapling_cache_mutex_off(s);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(01930)
                     "stapling_get_cached_response: cache miss");
        return;
    }
    if (resp_derlen <= 1) {
        /* should-not-occur; must have at least valid-when-stored flag +
         * OCSPResponseStatus
         */
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(01931)
                     "stapling_get_cached_response: response length invalid??");
        return;
    }
    p = resp_der;
    if (*p) /* valid when stored */
        *pok = TRUE;
    else
        *pok = FALSE;
    p++;
    resp_derlen--;
    rsp = d2i_OCSP_RESPONSE(NULL, &p, resp_derlen);
    if (!rsp) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(01932)
                     "stapling_get_cached_response: response parse error??");
        return;
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(01933)
                 "stapling_get_cached_response: cache hit");

    *prsp = rsp;
}