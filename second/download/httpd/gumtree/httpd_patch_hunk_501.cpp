         return ap_pass_brigade(f->next, bb);
     }
 
     ap_log_error(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, r->server,
                  "cache: running CACHE_OUT filter");
 
+    /* restore status of cached response */
+    r->status = cache->handle->status;
+
     /* recall_headers() was called in cache_select_url() */
     cache->provider->recall_body(cache->handle, r->pool, bb);
 
     /* This filter is done once it has served up its content */
     ap_remove_output_filter(f);
 
