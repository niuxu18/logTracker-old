     return OK;
 }
 
 int ap_open_logs(apr_pool_t *pconf, apr_pool_t *p /* plog */, 
                  apr_pool_t *ptemp, server_rec *s_main)
 {
-    apr_status_t rc = APR_SUCCESS;
+    apr_pool_t *stderr_p;
     server_rec *virt, *q;
     int replace_stderr;
-    apr_file_t *errfile = NULL;
 
+
+    /* Register to throw away the read_handles list when we
+     * cleanup plog.  Upon fork() for the apache children,
+     * this read_handles list is closed so only the parent
+     * can relaunch a lost log child.  These read handles 
+     * are always closed on exec.
+     * We won't care what happens to our stderr log child 
+     * between log phases, so we don't mind losing stderr's 
+     * read_handle a little bit early.
+     */
     apr_pool_cleanup_register(p, NULL, clear_handle_list,
                               apr_pool_cleanup_null);
-    if (open_error_log(s_main, p) != OK) {
+
+    /* HERE we need a stdout log that outlives plog.
+     * We *presume* the parent of plog is a process 
+     * or global pool which spans server restarts.
+     * Create our stderr_pool as a child of the plog's
+     * parent pool.
+     */
+    apr_pool_create(&stderr_p, apr_pool_parent_get(p));
+    apr_pool_tag(stderr_p, "stderr_pool");
+    
+    if (open_error_log(s_main, 1, stderr_p) != OK) {
         return DONE;
     }
 
     replace_stderr = 1;
     if (s_main->error_log) {
-        /* replace stderr with this new log */
+        apr_status_t rv;
+        
+        /* Replace existing stderr with new log. */
         apr_file_flush(s_main->error_log);
-        if ((rc = apr_file_open_stderr(&errfile, p)) == APR_SUCCESS) {
-            rc = apr_file_dup2(errfile, s_main->error_log, p);
-        }
-        if (rc != APR_SUCCESS) {
-            ap_log_error(APLOG_MARK, APLOG_CRIT, rc, s_main,
+        rv = apr_file_dup2(stderr_log, s_main->error_log, stderr_p);
+        if (rv != APR_SUCCESS) {
+            ap_log_error(APLOG_MARK, APLOG_CRIT, rv, s_main,
                          "unable to replace stderr with error_log");
         }
         else {
+            /* We are done with stderr_pool, close it, killing
+             * the previous generation's stderr logger
+             */
+            if (stderr_pool)
+                apr_pool_destroy(stderr_pool);
+            stderr_pool = stderr_p;
             replace_stderr = 0;
         }
     }
     /* note that stderr may still need to be replaced with something
      * because it points to the old error log, or back to the tty
      * of the submitter.
