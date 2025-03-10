    apr_status_t rv;

    if (socache_mc_id2key(ctx, id, idlen, buf, sizeof buf)) {
        return APR_EINVAL;
    }

    /* In APR-util - unclear what 'timeout' is, as it was not implemented */
    rv = apr_memcache_set(ctx->mc, buf, (char*)ucaData, nData, 0, 0);

    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s, APLOGNO(00790)
                     "scache_mc: error setting key '%s' "
                     "with %d bytes of data", buf, nData);
        return rv;
