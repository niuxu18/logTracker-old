      * If that changes, the driver DSO could be registered to unload against
      * our pool, which is probably not what we want.  Error checking isn't
      * necessary now, but in case that changes in the future ...
      */
     rv = apr_dbd_get_driver(rec->pool, cfg->name, &rec->driver);
     if (rv != APR_SUCCESS) {
-        switch (rv) {
-        case APR_ENOTIMPL:
+        if (APR_STATUS_IS_ENOTIMPL(rv)) {
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
                          "DBD: driver for %s not available", cfg->name);
-            break;
-        case APR_EDSOOPEN:
+        }
+        else if (APR_STATUS_IS_EDSOOPEN(rv)) {
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
                          "DBD: can't find driver for %s", cfg->name);
-            break;
-        case APR_ESYMNOTFOUND:
+        }
+        else if (APR_STATUS_IS_ESYMNOTFOUND(rv)) {
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
                          "DBD: driver for %s is invalid or corrupted",
                          cfg->name);
-            break;
-        default:
+        }
+        else {
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
                          "DBD: mod_dbd not compatible with APR in get_driver");
-            break;
         }
-
         apr_pool_destroy(rec->pool);
         return rv;
     }
 
-    rv = apr_dbd_open(rec->driver, rec->pool, cfg->params, &rec->handle);
+    rv = apr_dbd_open_ex(rec->driver, rec->pool, cfg->params, &rec->handle, &err);
     if (rv != APR_SUCCESS) {
         switch (rv) {
         case APR_EGENERAL:
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
-                         "DBD: Can't connect to %s", cfg->name);
+                         "DBD: Can't connect to %s: %s", cfg->name, err);
             break;
         default:
             ap_log_error(APLOG_MARK, APLOG_ERR, rv, cfg->server,
                          "DBD: mod_dbd not compatible with APR in open");
             break;
         }
