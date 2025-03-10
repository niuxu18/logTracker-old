 }
 
 apr_status_t h2_workers_register(h2_workers *workers, struct h2_mplx *m)
 {
     apr_status_t status = apr_thread_mutex_lock(workers->lock);
     if (status == APR_SUCCESS) {
-        ap_log_error(APLOG_MARK, APLOG_TRACE2, status, workers->s,
-                     "h2_workers: register mplx(%ld)", m->id);
+        ap_log_error(APLOG_MARK, APLOG_TRACE3, status, workers->s,
+                     "h2_workers: register mplx(%ld), idle=%d", 
+                     m->id, workers->idle_workers);
         if (in_list(workers, m)) {
             status = APR_EAGAIN;
         }
         else {
             H2_MPLX_LIST_INSERT_TAIL(&workers->mplxs, m);
+            ++workers->mplx_count;
             status = APR_SUCCESS;
         }
         
-        if (workers->idle_worker_count > 0) { 
+        if (workers->idle_workers > 0) { 
             apr_thread_cond_signal(workers->mplx_added);
         }
-        else if (workers->worker_count < workers->max_size) {
-            ap_log_error(APLOG_MARK, APLOG_TRACE1, 0, workers->s,
+        else if (status == APR_SUCCESS 
+                 && workers->worker_count < workers->max_workers) {
+            ap_log_error(APLOG_MARK, APLOG_TRACE3, 0, workers->s,
                          "h2_workers: got %d worker, adding 1", 
                          workers->worker_count);
             add_worker(workers);
         }
-        
-        /* cleanup any zombie workers that may have accumulated */
-        cleanup_zombies(workers, 0);
-        
         apr_thread_mutex_unlock(workers->lock);
     }
     return status;
 }
 
 apr_status_t h2_workers_unregister(h2_workers *workers, struct h2_mplx *m)
