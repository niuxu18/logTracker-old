     }
     else {
         /* Single process mode - this lock doesn't even need to exist */
         rv = apr_proc_mutex_create(&start_mutex, signal_name_prefix,
                                    APR_LOCK_DEFAULT, s->process->pool);
         if (rv != APR_SUCCESS) {
-            ap_log_error(APLOG_MARK,APLOG_ERR, rv, ap_server_conf,
-                         "%s child %d: Unable to init the start_mutex.",
-                         service_name, my_pid);
+            ap_log_error(APLOG_MARK,APLOG_ERR, rv, ap_server_conf, APLOGNO(00452)
+                         "%s child: Unable to init the start_mutex.",
+                         service_name);
             exit(APEXIT_CHILDINIT);
         }
 
         /* Borrow the shutdown_even as our _child_ loop exit event */
         exit_event = shutdown_event;
     }
