     }
 
     /* advance to the next generation */
     /* XXX: we really need to make sure this new generation number isn't in
      * use by any of the children.
      */
-    ++my_generation;
-    ap_scoreboard_image->global->running_generation = my_generation;
+    ++retained->my_generation;
+    ap_scoreboard_image->global->running_generation = retained->my_generation;
 
-    if (is_graceful) {
-        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf,
+    if (retained->is_graceful) {
+        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00493)
                      AP_SIG_GRACEFUL_STRING
                      " received.  Doing graceful restart");
         /* wake up the children...time to die.  But we'll have more soon */
         ap_event_pod_killpg(pod, ap_daemons_limit, TRUE);
 
 
