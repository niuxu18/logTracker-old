         return "Scope ids are not supported";
     }
     if (!port && !wild_port) {
         port = default_port;
     }
 
-    if (strcmp(host, "*") == 0) {
-        rv = apr_sockaddr_info_get(&my_addr, "0.0.0.0", APR_INET, port, 0, p);
-        if (rv) {
-            return "Could not resolve address '0.0.0.0' -- "
-                "check resolver configuration.";
-        }
-    }
-    else if (strcasecmp(host, "_default_") == 0
-        || strcmp(host, "255.255.255.255") == 0) {
-        rv = apr_sockaddr_info_get(&my_addr, "255.255.255.255", APR_INET, port, 0, p);
+    if (strcmp(host, "*") == 0 || strcasecmp(host, "_default_") == 0) {
+        rv = apr_sockaddr_info_get(&my_addr, NULL, APR_UNSPEC, port, 0, p);
         if (rv) {
-            return "Could not resolve address '255.255.255.255' -- "
+            return "Could not determine a wildcard address ('0.0.0.0') -- "
                 "check resolver configuration.";
         }
     }
     else {
         rv = apr_sockaddr_info_get(&my_addr, host, APR_UNSPEC, port, 0, p);
         if (rv != APR_SUCCESS) {
-            ap_log_error(APLOG_MARK, APLOG_ERR, rv, NULL,
+            ap_log_error(APLOG_MARK, APLOG_ERR, rv, NULL, APLOGNO(00547)
                 "Could not resolve host name %s -- ignoring!", host);
             return NULL;
         }
     }
 
     /* Remember all addresses for the host */
