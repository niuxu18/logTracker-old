     }
 
     /* Step 3: Read records from the back end server and handle them. */
     rv = dispatch(conn, conf, r, temp_pool, request_id,
                   &err, &bad_request, &has_responded);
     if (rv != APR_SUCCESS) {
+        /* If the client aborted the connection during retrieval or (partially)
+         * sending the response, don't return a HTTP_SERVICE_UNAVAILABLE, since
+         * this is not a backend problem. */
+        if (r->connection->aborted) {
+            ap_log_rerror(APLOG_MARK, APLOG_TRACE1, rv, r, 
+                          "The client aborted the connection.");
+            conn->close = 1;
+            return OK;
+        }
+
         ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01075)
                       "Error dispatching request to %s: %s%s%s",
                       server_portstr,
                       err ? "(" : "",
                       err ? err : "",
                       err ? ")" : "");
