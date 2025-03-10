                           input->task->id);
             return status;
         }
     }
     
     if ((bblen == 0) && input->task->input_eos) {
+        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
+                      "h2_task_input(%s): eos", input->task->id);
         return APR_EOF;
     }
     
-    while ((bblen == 0) || (mode == AP_MODE_READBYTES && bblen < readbytes)) {
+    while (bblen == 0) {
         /* Get more data for our stream from mplx.
          */
         ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                       "h2_task_input(%s): get more data from mplx, block=%d, "
                       "readbytes=%ld, queued=%ld",
                       input->task->id, block, 
                       (long)readbytes, (long)bblen);
         
-        /* Although we sometimes get called with APR_NONBLOCK_READs, 
-         we seem to  fill our buffer blocking. Otherwise we get EAGAIN,
-         return that to our caller and everyone throws up their hands,
-         never calling us again. */
-        status = h2_mplx_in_read(input->task->mplx, APR_BLOCK_READ,
+        /* Override the block mode we get called with depending on the input's
+         * setting. 
+         */
+        status = h2_mplx_in_read(input->task->mplx, block,
                                  input->task->stream_id, input->bb, 
+                                 f->r? f->r->trailers_in : NULL, 
                                  input->task->io);
         ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                       "h2_task_input(%s): mplx in read returned",
                       input->task->id);
-        if (status != APR_SUCCESS) {
+        if (APR_STATUS_IS_EAGAIN(status) 
+            && (mode == AP_MODE_GETLINE || block == APR_BLOCK_READ)) {
+            /* chunked input handling does not seem to like it if we
+             * return with APR_EAGAIN from a GETLINE read... 
+             * upload 100k test on test-ser.example.org hangs */
+            status = APR_SUCCESS;
+        }
+        else if (status != APR_SUCCESS) {
             return status;
         }
+        
         status = apr_brigade_length(input->bb, 1, &bblen);
         if (status != APR_SUCCESS) {
             return status;
         }
-        if ((bblen == 0) && (block == APR_NONBLOCK_READ)) {
-            return h2_util_has_eos(input->bb, -1)? APR_EOF : APR_EAGAIN;
-        }
+        
         ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                       "h2_task_input(%s): mplx in read, %ld bytes in brigade",
                       input->task->id, (long)bblen);
+        if (h2_task_logio_add_bytes_in) {
+            h2_task_logio_add_bytes_in(f->c, bblen);
+        }
     }
     
     ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                   "h2_task_input(%s): read, mode=%d, block=%d, "
                   "readbytes=%ld, queued=%ld",
                   input->task->id, mode, block, 
                   (long)readbytes, (long)bblen);
            
     if (!APR_BRIGADE_EMPTY(input->bb)) {
         if (mode == AP_MODE_EXHAUSTIVE) {
             /* return all we have */
-            return h2_util_move(bb, input->bb, readbytes, NULL, 
-                                "task_input_read(exhaustive)");
+            status = h2_util_move(bb, input->bb, readbytes, NULL, 
+                                  "task_input_read(exhaustive)");
         }
         else if (mode == AP_MODE_READBYTES) {
-            return h2_util_move(bb, input->bb, readbytes, NULL, 
-                                "task_input_read(readbytes)");
+            status = h2_util_move(bb, input->bb, readbytes, NULL, 
+                                  "task_input_read(readbytes)");
         }
         else if (mode == AP_MODE_SPECULATIVE) {
             /* return not more than was asked for */
-            return h2_util_copy(bb, input->bb, readbytes,  
-                                "task_input_read(speculative)");
+            status = h2_util_copy(bb, input->bb, readbytes,  
+                                  "task_input_read(speculative)");
         }
         else if (mode == AP_MODE_GETLINE) {
             /* we are reading a single LF line, e.g. the HTTP headers */
             status = apr_brigade_split_line(bb, input->bb, block, 
                                             HUGE_STRING_LEN);
             if (APLOGctrace1(f->c)) {
