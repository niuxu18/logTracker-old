     if(strcmp(r->handler,ASIS_MAGIC_TYPE) && strcmp(r->handler,"send-as-is"))
         return DECLINED;
 
     r->allowed |= (AP_METHOD_BIT << M_GET);
     if (r->method_number != M_GET)
         return DECLINED;
-    if (r->finfo.filetype == 0) {
+    if (r->finfo.filetype == APR_NOFILE) {
         ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                     "File does not exist: %s", r->filename);
         return HTTP_NOT_FOUND;
     }
 
     if ((rv = apr_file_open(&f, r->filename, APR_READ,
