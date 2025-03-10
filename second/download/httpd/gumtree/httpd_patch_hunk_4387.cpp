             }
             break;
             
         case H2_PROXYS_ST_BUSY:
         case H2_PROXYS_ST_LOCAL_SHUTDOWN:
         case H2_PROXYS_ST_REMOTE_SHUTDOWN:
-            while (nghttp2_session_want_write(session->ngh2)) {
-                int rv = nghttp2_session_send(session->ngh2);
-                if (rv < 0 && nghttp2_is_fatal(rv)) {
-                    ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, session->c, 
-                                  "h2_proxy_session(%s): write, rv=%d", session->id, rv);
-                    dispatch_event(session, H2_PROXYS_EV_CONN_ERROR, rv, NULL);
-                    break;
-                }
-                have_written = 1;
-            }
+            have_written = send_loop(session);
             
             if (nghttp2_session_want_read(session->ngh2)) {
                 status = h2_proxy_session_read(session, 0, 0);
                 if (status == APR_SUCCESS) {
                     have_read = 1;
                 }
