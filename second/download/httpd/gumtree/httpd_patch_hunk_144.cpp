  */
 static int dav_handler(request_rec *r)
 {
     if (strcmp(r->handler, DAV_HANDLER_NAME) != 0)
         return DECLINED;
 
+    /* Reject requests with an unescaped hash character, as these may
+     * be more destructive than the user intended. */
+    if (r->parsed_uri.fragment != NULL) {
+        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
+                     "buggy client used un-escaped hash in Request-URI");
+        return dav_error_response(r, HTTP_BAD_REQUEST, 
+                                  "The request was invalid: the URI included "
+                                  "an un-escaped hash character");
+    }
+
     /* ### do we need to do anything with r->proxyreq ?? */
 
     /*
      * ### anything else to do here? could another module and/or
      * ### config option "take over" the handler here? i.e. how do
      * ### we lock down this hierarchy so that we are the ultimate
