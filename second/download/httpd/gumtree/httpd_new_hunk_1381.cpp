    apr_table_entry_t *end_elt;
    apr_uint32_t checksum;
    int hash;

#if APR_POOL_DEBUG
    {
	apr_pool_t *pool;
	pool = apr_pool_find(key);
	if ((pool != key) && (!apr_pool_is_ancestor(pool, t->a.pool))) {
	    fprintf(stderr, "apr_table_mergen: key not in ancestor pool of t\n");
	    abort();
	}
	pool = apr_pool_find(val);
	if ((pool != val) && (!apr_pool_is_ancestor(pool, t->a.pool))) {
	    fprintf(stderr, "apr_table_mergen: val not in ancestor pool of t\n");
	    abort();
	}
    }
#endif

