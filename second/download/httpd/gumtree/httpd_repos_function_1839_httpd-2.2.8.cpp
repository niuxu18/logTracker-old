static int uldap_cache_getuserdn(request_rec *r, util_ldap_connection_t *ldc,
                                 const char *url, const char *basedn,
                                 int scope, char **attrs, const char *filter,
                                 const char **binddn, const char ***retvals)
{
    const char **vals = NULL;
    int numvals = 0;
    int result = 0;
    LDAPMessage *res, *entry;
    char *dn;
    int count;
    int failures = 0;
    util_url_node_t *curl;              /* Cached URL node */
    util_url_node_t curnode;
    util_search_node_t *search_nodep;   /* Cached search node */
    util_search_node_t the_search_node;
    apr_time_t curtime;

    util_ldap_state_t *st =
        (util_ldap_state_t *)ap_get_module_config(r->server->module_config,
        &ldap_module);

    /* Get the cache node for this url */
    LDAP_CACHE_LOCK();
    curnode.url = url;
    curl = (util_url_node_t *)util_ald_cache_fetch(st->util_ldap_cache,
                                                   &curnode);
    if (curl == NULL) {
        curl = util_ald_create_caches(st, url);
    }
    LDAP_CACHE_UNLOCK();

    if (curl) {
        LDAP_CACHE_LOCK();
        the_search_node.username = filter;
        search_nodep = util_ald_cache_fetch(curl->search_cache,
                                            &the_search_node);
        if (search_nodep != NULL) {

            /* found entry in search cache... */
            curtime = apr_time_now();

            /*
             * Remove this item from the cache if its expired.
             */
            if ((curtime - search_nodep->lastbind) > st->search_cache_ttl) {
                /* ...but entry is too old */
                util_ald_cache_remove(curl->search_cache, search_nodep);
            }
            else {
                /* ...and entry is valid */
                *binddn = apr_pstrdup(r->pool, search_nodep->dn);
                if (attrs) {
                    int i = 0, k = 0;
                    while (attrs[k++]);
                    *retvals = apr_pcalloc(r->pool, sizeof(char *) * k);
                    while (search_nodep->vals[i]) {
                        (*retvals)[i] = apr_pstrdup(r->pool, search_nodep->vals[i]);
                        i++;
                    }
                }
                LDAP_CACHE_UNLOCK();
                ldc->reason = "Search successful (cached)";
                return LDAP_SUCCESS;
            }
        }
        /* unlock this read lock */
        LDAP_CACHE_UNLOCK();
    }

    /*
     * At this point, there is no valid cached search, so lets do the search.
     */

    /*
     * If LDAP operation fails due to LDAP_SERVER_DOWN, control returns here.
     */
start_over:
    if (failures++ > 10) {
        return result;
    }
    if (LDAP_SUCCESS != (result = uldap_connection_open(r, ldc))) {
        return result;
    }

    /* try do the search */
    result = ldap_search_ext_s(ldc->ldap,
                               (char *)basedn, scope,
                               (char *)filter, attrs, 0,
                               NULL, NULL, NULL, APR_LDAP_SIZELIMIT, &res);
    if (AP_LDAP_IS_SERVER_DOWN(result))
    {
        ldc->reason = "ldap_search_ext_s() for user failed with server down";
        uldap_connection_unbind(ldc);
        goto start_over;
    }

    /* if there is an error (including LDAP_NO_SUCH_OBJECT) return now */
    if (result != LDAP_SUCCESS) {
        ldc->reason = "ldap_search_ext_s() for user failed";
        return result;
    }

    /*
     * We should have found exactly one entry; to find a different
     * number is an error.
     */
    count = ldap_count_entries(ldc->ldap, res);
    if (count != 1)
    {
        if (count == 0 )
            ldc->reason = "User not found";
        else
            ldc->reason = "User is not unique (search found two "
                          "or more matches)";
        ldap_msgfree(res);
        return LDAP_NO_SUCH_OBJECT;
    }

    entry = ldap_first_entry(ldc->ldap, res);

    /* Grab the dn, copy it into the pool, and free it again */
    dn = ldap_get_dn(ldc->ldap, entry);
    *binddn = apr_pstrdup(r->pool, dn);
    ldap_memfree(dn);

    /*
     * Get values for the provided attributes.
     */
    if (attrs) {
        int k = 0;
        int i = 0;
        while (attrs[k++]);
        vals = apr_pcalloc(r->pool, sizeof(char *) * (k+1));
        numvals = k;
        while (attrs[i]) {
            char **values;
            int j = 0;
            char *str = NULL;
            /* get values */
            values = ldap_get_values(ldc->ldap, entry, attrs[i]);
            while (values && values[j]) {
                str = str ? apr_pstrcat(r->pool, str, "; ", values[j], NULL)
                          : apr_pstrdup(r->pool, values[j]);
                j++;
            }
            ldap_value_free(values);
            vals[i] = str;
            i++;
        }
        *retvals = vals;
    }

    /*
     * Add the new username to the search cache.
     */
    if (curl) {
        LDAP_CACHE_LOCK();
        the_search_node.username = filter;
        the_search_node.dn = *binddn;
        the_search_node.bindpw = NULL;
        the_search_node.lastbind = apr_time_now();
        the_search_node.vals = vals;
        the_search_node.numvals = numvals;

        /* Search again to make sure that another thread didn't ready insert
         * this node into the cache before we got here. If it does exist then
         * update the lastbind
         */
        search_nodep = util_ald_cache_fetch(curl->search_cache,
                                            &the_search_node);
        if ((search_nodep == NULL) ||
            (strcmp(*binddn, search_nodep->dn) != 0)) {

            /* Nothing in cache, insert new entry */
            util_ald_cache_insert(curl->search_cache, &the_search_node);
        }
        /*
         * Don't update lastbind on entries with bindpw because
         * we haven't verified that password. It's OK to update
         * the entry if there is no password in it.
         */
        else if (!search_nodep->bindpw) {
            /* Cache entry is valid, update lastbind */
            search_nodep->lastbind = the_search_node.lastbind;
        }
        LDAP_CACHE_UNLOCK();
    }

    ldap_msgfree(res);

    ldc->reason = "Search successful";
    return LDAP_SUCCESS;
}