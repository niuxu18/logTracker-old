     if (retained->module_loads == 1) {
         startup = 1;
     }
 
     if (server_limit > MAX_SERVER_LIMIT) {
         if (startup) {
-            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL,
+            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00497)
                          "WARNING: ServerLimit of %d exceeds compile-time "
                          "limit of", server_limit);
             ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL,
                          " %d servers, decreasing to %d.",
                          MAX_SERVER_LIMIT, MAX_SERVER_LIMIT);
         } else {
-            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
+            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00498)
                          "ServerLimit of %d exceeds compile-time limit "
                          "of %d, decreasing to match",
                          server_limit, MAX_SERVER_LIMIT);
         }
         server_limit = MAX_SERVER_LIMIT;
     }
     else if (server_limit < 1) {
         if (startup) {
-            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL,
+            ap_log_error(APLOG_MARK, APLOG_WARNING | APLOG_STARTUP, 0, NULL, APLOGNO(00499)
                          "WARNING: ServerLimit of %d not allowed, "
                          "increasing to 1.", server_limit);
         } else {
-            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s,
+            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(00500)
                          "ServerLimit of %d not allowed, increasing to 1",
                          server_limit);
         }
         server_limit = 1;
     }
 
