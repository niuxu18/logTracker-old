             ap_log_cerror(APLOG_MARK, APLOG_ERR, APR_EGENERAL, c,
                           APLOGNO(02929) "nghttp2_session_server_new: %s",
                           nghttp2_strerror(rv));
             h2_session_destroy(session);
             return NULL;
         }
-        
+         
+        n = h2_config_geti(session->config, H2_CONF_PUSH_DIARY_SIZE);
+        session->push_diary = h2_push_diary_create(session->pool, n);
+        
+        if (APLOGcdebug(c)) {
+            ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c, APLOGNO(03200)
+                          "h2_session(%ld) created, max_streams=%d, "
+                          "stream_mem=%d, workers_limit=%d, workers_max=%d, "
+                          "push_diary(type=%d,N=%d)",
+                          session->id, (int)session->max_stream_count, 
+                          (int)session->max_stream_mem,
+                          session->mplx->workers_limit, 
+                          session->mplx->workers_max, 
+                          session->push_diary->dtype, 
+                          (int)session->push_diary->N);
+        }
     }
     return session;
 }
 
-h2_session *h2_session_create(conn_rec *c, const h2_config *config, 
-                              h2_workers *workers)
-{
-    return h2_session_create_int(c, NULL, config, workers);
-}
-
-h2_session *h2_session_rcreate(request_rec *r, const h2_config *config, 
-                               h2_workers *workers)
-{
-    return h2_session_create_int(r->connection, r, config, workers);
-}
-
-static void h2_session_cleanup(h2_session *session)
+h2_session *h2_session_create(conn_rec *c, h2_ctx *ctx, h2_workers *workers)
 {
-    AP_DEBUG_ASSERT(session);
-    /* This is an early cleanup of the session that may
-     * discard what is no longer necessary for *new* streams
-     * and general HTTP/2 processing.
-     * At this point, all frames are in transit or somehwere in
-     * our buffers or passed down output filters.
-     * h2 streams might still being written out.
-     */
-    if (session->ngh2) {
-        nghttp2_session_del(session->ngh2);
-        session->ngh2 = NULL;
-    }
-    if (session->spare) {
-        apr_pool_destroy(session->spare);
-        session->spare = NULL;
-    }
+    return h2_session_create_int(c, NULL, ctx, workers);
 }
 
-void h2_session_destroy(h2_session *session)
+h2_session *h2_session_rcreate(request_rec *r, h2_ctx *ctx, h2_workers *workers)
 {
-    AP_DEBUG_ASSERT(session);
-    h2_session_cleanup(session);
-    
-    if (session->mplx) {
-        h2_mplx_release_and_join(session->mplx, session->iowait);
-        session->mplx = NULL;
-    }
-    if (session->streams) {
-        if (!h2_stream_set_is_empty(session->streams)) {
-            ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, session->c,
-                          "h2_session(%ld): destroy, %d streams open",
-                          session->id, (int)h2_stream_set_size(session->streams));
-        }
-        h2_stream_set_destroy(session->streams);
-        session->streams = NULL;
-    }
-    if (session->pool) {
-        apr_pool_destroy(session->pool);
-    }
+    return h2_session_create_int(r->connection, r, ctx, workers);
 }
 
-
 void h2_session_eoc_callback(h2_session *session)
 {
     ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, session->c,
                   "session(%ld): cleanup and destroy", session->id);
     apr_pool_cleanup_kill(session->pool, session, session_pool_cleanup);
     h2_session_destroy(session);
