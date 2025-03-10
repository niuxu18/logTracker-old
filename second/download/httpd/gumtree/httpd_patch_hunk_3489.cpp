     const char *type = ap_auth_type(r);
     if (type) {
         ap_run_note_auth_failure(r, type);
     }
     else {
         ap_log_rerror(APLOG_MARK, APLOG_ERR,
-                      0, r, "need AuthType to note auth failure: %s", r->uri);
+                      0, r, APLOGNO(00571) "need AuthType to note auth failure: %s", r->uri);
     }
 }
 
 AP_DECLARE(void) ap_note_basic_auth_failure(request_rec *r)
 {
     ap_note_auth_failure(r);
