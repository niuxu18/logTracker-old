 /*******************************************************************************
  * Once per lifetime init, retrieve optional functions
  */
 apr_status_t h2_h2_init(apr_pool_t *pool, server_rec *s)
 {
     (void)pool;
-    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, "h2_h2, child_init");
+    ap_log_error(APLOG_MARK, APLOG_TRACE1, 0, s, "h2_h2, child_init");
     opt_ssl_engine_disable = APR_RETRIEVE_OPTIONAL_FN(ssl_engine_disable);
     opt_ssl_is_https = APR_RETRIEVE_OPTIONAL_FN(ssl_is_https);
     opt_ssl_var_lookup = APR_RETRIEVE_OPTIONAL_FN(ssl_var_lookup);
     
     if (!opt_ssl_is_https || !opt_ssl_var_lookup) {
         ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
