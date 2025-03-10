 /*  _________________________________________________________________
 **
 **  Module Initialization
 **  _________________________________________________________________
 */
 
-static char *ssl_add_version_component(apr_pool_t *p,
-                                       server_rec *s,
-                                       char *name)
-{
-    char *val = ssl_var_lookup(p, s, NULL, NULL, name);
-
-    if (val && *val) {
-        ap_add_version_component(p, val);
-    }
-
-    return val;
-}
-
-static char *version_components[] = {
-    "SSL_VERSION_PRODUCT",
-    "SSL_VERSION_INTERFACE",
-    "SSL_VERSION_LIBRARY",
-    NULL
-};
 
 static void ssl_add_version_components(apr_pool_t *p,
                                        server_rec *s)
 {
-    char *vals[sizeof(version_components)/sizeof(char *)];
-    int i;
+    char *modver = ssl_var_lookup(p, s, NULL, NULL, "SSL_VERSION_INTERFACE");
+    char *libver = ssl_var_lookup(p, s, NULL, NULL, "SSL_VERSION_LIBRARY");
+    char *incver = ssl_var_lookup(p, s, NULL, NULL, 
+                                  "SSL_VERSION_LIBRARY_INTERFACE");
 
-    for (i=0; version_components[i]; i++) {
-        vals[i] = ssl_add_version_component(p, s,
-                                            version_components[i]);
-    }
+    ap_add_version_component(p, modver);
+    ap_add_version_component(p, libver);
 
     ap_log_error(APLOG_MARK, APLOG_INFO, 0, s,
-                 "Server: %s, Interface: %s, Library: %s",
-                 AP_SERVER_BASEVERSION,
-                 vals[1],  /* SSL_VERSION_INTERFACE */
-                 vals[2]); /* SSL_VERSION_LIBRARY */
+                 "%s compiled against Server: %s, Library: %s",
+                 modver, AP_SERVER_BASEVERSION, incver);
 }
 
 
 /*
  * Handle the Temporary RSA Keys and DH Params
  */
