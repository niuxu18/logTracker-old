          */
         if (cid->completed) {
             (void)apr_thread_mutex_unlock(cid->completed);
             return 1;
         }
         else if (cid->dconf.log_unsupported) {
-            ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r,
+            ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(02671)
                           "ServerSupportFunction "
                           "HSE_REQ_DONE_WITH_SESSION is not supported: %s",
                           r->filename);
         }
         apr_set_os_error(APR_FROM_OS_ERROR(ERROR_INVALID_PARAMETER));
         return 0;
