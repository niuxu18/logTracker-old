         }
 
         if (status == APEXIT_CHILDFATAL) {
             ap_log_error(APLOG_MARK, APLOG_ALERT,
                          0, ap_server_conf,
                          "Child %" APR_PID_T_FMT
-                         " returned a Fatal error..." APR_EOL_STR
-                         "Apache is exiting!",
+                         " returned a Fatal error... Apache is exiting!",
                          pid->pid);
             return APEXIT_CHILDFATAL;
         }
 
         return 0;
     }
