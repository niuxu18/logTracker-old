     }
 
     if (thread_limit > MAX_THREAD_LIMIT) {
         if (startup) {
             ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00439)
                          "WARNING: ThreadLimit of %d exceeds compile-time "
-                         "limit of", thread_limit);
-            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL,
-                         " %d threads, decreasing to %d.",
-                         MAX_THREAD_LIMIT, MAX_THREAD_LIMIT);
+                         "limit of %d threads, decreasing to %d.",
+                         thread_limit, MAX_THREAD_LIMIT, MAX_THREAD_LIMIT);
         } else if (is_parent) {
             ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00440)
                          "ThreadLimit of %d exceeds compile-time limit "
                          "of %d, decreasing to match",
                          thread_limit, MAX_THREAD_LIMIT);
         }
