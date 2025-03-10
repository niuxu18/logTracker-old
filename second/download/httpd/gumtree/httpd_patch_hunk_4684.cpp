                  * A child created at the same time as a graceful happens 
                  * can find the lock missing and create a fatal error.
                  * It is not fatal for the last generation to be in this state.
                  */
                 if (child_slot < 0
                     || ap_get_scoreboard_process(child_slot)->generation
-                       == retained->my_generation) {
-                    shutdown_pending = 1;
+                       == retained->mpm->my_generation) {
+                    retained->mpm->shutdown_pending = 1;
                     child_fatal = 1;
                     return;
                 }
                 else {
                     ap_log_error(APLOG_MARK, APLOG_WARNING, 0, ap_server_conf, APLOGNO(00290)
                                  "Ignoring fatal error in child of previous "
