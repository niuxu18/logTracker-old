                 ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01224)
                               "Timeout during reading request entity data");
                 return HTTP_REQUEST_TIME_OUT;
             }
             ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01225)
                           "Error reading request entity data");
-            return HTTP_INTERNAL_SERVER_ERROR;
+            return ap_map_http_request_error(rv, HTTP_BAD_REQUEST);
         }
 
         for (bucket = APR_BRIGADE_FIRST(bb);
              bucket != APR_BRIGADE_SENTINEL(bb);
              bucket = APR_BUCKET_NEXT(bucket))
         {
