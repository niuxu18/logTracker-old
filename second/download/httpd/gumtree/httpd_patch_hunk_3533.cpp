             if (status != APR_SUCCESS) {
                 goto read_error;
             }
 
             total_read += len;
             if (limit_xml_body && total_read > limit_xml_body) {
-                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
+                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00539)
                               "XML request body is larger than the configured "
                               "limit of %lu", (unsigned long)limit_xml_body);
                 result = HTTP_REQUEST_ENTITY_TOO_LARGE;
                 goto read_error;
             }
 
