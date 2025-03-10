         apr_bucket *bucket;
 
         rv = ap_get_brigade(r->input_filters, bb, AP_MODE_READBYTES,
                             APR_BLOCK_READ, HUGE_STRING_LEN);
 
         if (rv != APR_SUCCESS) {
+            if (APR_STATUS_IS_TIMEUP(rv)) {
+                ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
+                              "Timeout during reading request entity data");
+                return HTTP_REQUEST_TIME_OUT;
+            }
             ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r,
                           "Error reading request entity data");
-            return ap_map_http_request_error(rv, HTTP_BAD_REQUEST);
+            return HTTP_INTERNAL_SERVER_ERROR;
         }
 
         for (bucket = APR_BRIGADE_FIRST(bb);
              bucket != APR_BRIGADE_SENTINEL(bb);
              bucket = APR_BUCKET_NEXT(bucket))
         {
