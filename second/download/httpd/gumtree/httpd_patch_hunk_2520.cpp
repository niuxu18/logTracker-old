         ap_remove_output_filter(next);
         next = next->next;
     }
 
     /* kick off the filter stack */
     out = apr_brigade_create(r->pool, r->connection->bucket_alloc);
-    rv = ap_pass_brigade(r->output_filters, out);
-    if (rv != APR_SUCCESS) {
-        if (rv != AP_FILTER_ERROR) {
-            ap_log_error(APLOG_MARK, APLOG_ERR, rv, r->server,
-                         "cache: error returned while trying to return %s "
-                         "cached data",
-                         cache->provider_name);
-        }
-        return rv;
-    }
+    e = apr_bucket_eos_create(out->bucket_alloc);
+    APR_BRIGADE_INSERT_TAIL(out, e);
 
-    return OK;
+    return ap_pass_brigade_fchk(r, out,
+                                "cache_quick_handler(%s): ap_pass_brigade returned",
+                                cache->provider_name);
 }
 
 /**
  * If the two filter handles are present within the filter chain, replace
  * the last instance of the first filter with the last instance of the
  * second filter, and return true. If the second filter is not present at
  * all, the first filter is removed, and false is returned. If neither
  * filter is present, false is returned and this function does nothing.
+ * If a stop filter is specified, processing will stop once this filter is
+ * reached.
  */
 static int cache_replace_filter(ap_filter_t *next, ap_filter_rec_t *from,
-        ap_filter_rec_t *to) {
+        ap_filter_rec_t *to, ap_filter_rec_t *stop) {
     ap_filter_t *ffrom = NULL, *fto = NULL;
-    while (next) {
-        if (next->frec == from && !next->ctx) {
+    while (next && next->frec != stop) {
+        if (next->frec == from) {
             ffrom = next;
         }
-        if (next->frec == to && !next->ctx) {
+        if (next->frec == to) {
             fto = next;
         }
         next = next->next;
     }
     if (ffrom && fto) {
         ffrom->frec = fto->frec;
