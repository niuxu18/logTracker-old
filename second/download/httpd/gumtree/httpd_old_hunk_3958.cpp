                    h->cache_obj->key);
            apr_pool_destroy(sobj->pool);
            sobj->pool = NULL;
            return APR_EGENERAL;
        }
        if (cl_header) {
            apr_int64_t cl = apr_atoi64(cl_header);
            if ((errno == 0) && (sobj->file_size != cl)) {
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(02381)
                        "URL %s didn't receive complete response, not caching",
                        h->cache_obj->key);
                apr_pool_destroy(sobj->pool);
                sobj->pool = NULL;
                return APR_EGENERAL;
