                             APR_OS_DEFAULT, p)) != APR_SUCCESS) {
         ap_log_error(APLOG_MARK, APLOG_STARTUP, rc, NULL,
                      "%s: could not open error log file %s.",
                      ap_server_argv0, fname);
         return rc;
     }
-    if ((rc = apr_file_open_stderr(&stderr_log, p)) == APR_SUCCESS) {
+    if (!stderr_pool) {
+        /* This is safe provided we revert it when we are finished.
+         * We don't manager the callers pool!
+         */
+        stderr_pool = p;
+    }
+    if ((rc = apr_file_open_stderr(&stderr_log, stderr_pool)) 
+            == APR_SUCCESS) {
         apr_file_flush(stderr_log);
-        if ((rc = apr_file_dup2(stderr_log, stderr_file, p)) == APR_SUCCESS) {
+        if ((rc = apr_file_dup2(stderr_log, stderr_file, stderr_pool)) 
+                == APR_SUCCESS) {
             apr_file_close(stderr_file);
+            /*
+             * You might ponder why stderr_pool should survive?
+             * The trouble is, stderr_pool may have s_main->error_log,
+             * so we aren't in a position to destory stderr_pool until
+             * the next recycle.  There's also an apparent bug which 
+             * is not; if some folk decided to call this function before 
+             * the core open error logs hook, this pool won't survive.
+             * Neither does the stderr logger, so this isn't a problem.
+             */
         }
     }
+    /* Revert, see above */
+    if (stderr_pool == p)
+        stderr_pool = NULL;
+
     if (rc != APR_SUCCESS) {
         ap_log_error(APLOG_MARK, APLOG_CRIT, rc, NULL,
-                     "unable to replace stderr with error_log");
+                     "unable to replace stderr with error log file");
     }
     return rc;
 }
 
 static void log_child_errfn(apr_pool_t *pool, apr_status_t err,
                             const char *description)
