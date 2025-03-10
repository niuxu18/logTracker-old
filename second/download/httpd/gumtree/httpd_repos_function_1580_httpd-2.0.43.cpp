int util_ldap_cache_comparedn(request_rec *r, util_ldap_connection_t *ldc, 
                            const char *url, const char *dn, const char *reqdn, 
                            int compare_dn_on_server)
{
    int result = 0;
    util_url_node_t *curl; 
    util_url_node_t curnode;
    util_dn_compare_node_t *node;
    util_dn_compare_node_t newnode;
    int failures = 0;
    LDAPMessage *res, *entry;
    char *searchdn;

    util_ldap_state_t *st = 
        (util_ldap_state_t *)ap_get_module_config(r->server->module_config,
        &ldap_module);

    /* read lock this function */
    LDAP_CACHE_LOCK_CREATE(st->pool);

    /* get cache entry (or create one) */
    LDAP_CACHE_WRLOCK();

    curnode.url = url;
    curl = util_ald_cache_fetch(util_ldap_cache, &curnode);
    if (curl == NULL) {
        curl = util_ald_create_caches(st, url);
    }
    LDAP_CACHE_UNLOCK();

    /* a simple compare? */
    if (!compare_dn_on_server) {
        /* unlock this read lock */
        if (strcmp(dn, reqdn)) {
            ldc->reason = "DN Comparison FALSE (direct strcmp())";
            return LDAP_COMPARE_FALSE;
        }
        else {
            ldc->reason = "DN Comparison TRUE (direct strcmp())";
            return LDAP_COMPARE_TRUE;
        }
    }

    /* no - it's a server side compare */
    LDAP_CACHE_RDLOCK();

    /* is it in the compare cache? */
    newnode.reqdn = (char *)reqdn;
    node = util_ald_cache_fetch(curl->dn_compare_cache, &newnode);
    if (node != NULL) {
        /* If it's in the cache, it's good */
        /* unlock this read lock */
        LDAP_CACHE_UNLOCK();
        ldc->reason = "DN Comparison TRUE (cached)";
        return LDAP_COMPARE_TRUE;
    }

    /* unlock this read lock */
    LDAP_CACHE_UNLOCK();

start_over:
    if (failures++ > 10) {
	/* too many failures */
        return result;
    }

    /* make a server connection */
    if (LDAP_SUCCESS != (result = util_ldap_connection_open(ldc))) {
	/* connect to server failed */
        return result;
    }

    /* search for reqdn */
    if ((result = ldap_search_ext_s(ldc->ldap, const_cast(reqdn), LDAP_SCOPE_BASE, 
				    "(objectclass=*)", NULL, 1, 
				    NULL, NULL, NULL, -1, &res)) == LDAP_SERVER_DOWN) {
        util_ldap_connection_close(ldc);
        ldc->reason = "DN Comparison ldap_search_ext_s() failed with server down";
        goto start_over;
    }
    if (result != LDAP_SUCCESS) {
        /* search for reqdn failed - no match */
        ldc->reason = "DN Comparison ldap_search_ext_s() failed";
        return result;
    }

    entry = ldap_first_entry(ldc->ldap, res);
    searchdn = ldap_get_dn(ldc->ldap, entry);

    ldap_msgfree(res);
    if (strcmp(dn, searchdn) != 0) {
        /* compare unsuccessful */
        ldc->reason = "DN Comparison FALSE (checked on server)";
        result = LDAP_COMPARE_FALSE;
    }
    else {
        /* compare successful - add to the compare cache */
        LDAP_CACHE_RDLOCK();
        newnode.reqdn = (char *)reqdn;
        newnode.dn = (char *)dn;
        util_ald_cache_insert(curl->dn_compare_cache, &newnode);
        LDAP_CACHE_UNLOCK();
        ldc->reason = "DN Comparison TRUE (checked on server)";
        result = LDAP_COMPARE_TRUE;
    }
    ldap_memfree(searchdn);
    return result;

}