    {
	if (!apr_pool_is_ancestor(apr_pool_find(key), t->a.pool)) {
	    fprintf(stderr, "apr_table_mergen: key not in ancestor pool of t\n");
	    abort();
	}
	if (!apr_pool_is_ancestor(apr_pool_find(val), t->a.pool)) {
	    fprintf(stderr, "apr_table_mergen: key not in ancestor pool of t\n");
	    abort();
	}
    }
#endif

    COMPUTE_KEY_CHECKSUM(key, checksum);
