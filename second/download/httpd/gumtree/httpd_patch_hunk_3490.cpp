 
     if (!(t = ap_auth_type(r)) || strcasecmp(t, "Basic"))
         return DECLINED;
 
     if (!ap_auth_name(r)) {
         ap_log_rerror(APLOG_MARK, APLOG_ERR,
-                      0, r, "need AuthName: %s", r->uri);
+                      0, r, APLOGNO(00572) "need AuthName: %s", r->uri);
         return HTTP_INTERNAL_SERVER_ERROR;
     }
 
     if (!auth_line) {
         ap_note_auth_failure(r);
         return HTTP_UNAUTHORIZED;
     }
 
     if (strcasecmp(ap_getword(r->pool, &auth_line, ' '), "Basic")) {
         /* Client tried to authenticate using wrong auth scheme */
-        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
+        ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r, APLOGNO(00573)
                       "client used wrong authentication scheme: %s", r->uri);
         ap_note_auth_failure(r);
         return HTTP_UNAUTHORIZED;
     }
 
     while (*auth_line == ' ' || *auth_line == '\t') {
