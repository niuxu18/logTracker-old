   }
 }
 
 /* Curl_failf() is for messages stating why we failed, the LAST one will be
    returned for the user (if requested) */
 
-void Curl_failf(struct UrlData *data, char *fmt, ...)
+void Curl_failf(struct UrlData *data, const char *fmt, ...)
 {
   va_list ap;
   va_start(ap, fmt);
   if(data->errorbuffer)
     vsnprintf(data->errorbuffer, CURL_ERROR_SIZE, fmt, ap);
   va_end(ap);
 }
 
 /* Curl_sendf() sends formated data to the server */
 size_t Curl_sendf(int sockfd, struct connectdata *conn,
-                  char *fmt, ...)
+                  const char *fmt, ...)
 {
   struct UrlData *data = conn->data;
   size_t bytes_written;
   char *s;
   va_list ap;
   va_start(ap, fmt);
