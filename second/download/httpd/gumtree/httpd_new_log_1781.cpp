ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
                         "disk_cache: Error when writing cache file for URL %s",
                         h->cache_obj->key);