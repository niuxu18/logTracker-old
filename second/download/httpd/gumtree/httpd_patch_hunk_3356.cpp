         /* terminate the free list */
         if (free_length == 0) {
             /* only report this condition once */
             static int reported = 0;
 
             if (!reported) {
-                ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf,
-                    "server reached MaxClients setting, consider"
-                    " raising the MaxClients setting");
+                ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO(00220)
+                    "server reached MaxRequestWorkers setting, consider"
+                    " raising the MaxRequestWorkers setting");
                 reported = 1;
             }
             idle_spawn_rate = 1;
         }
         else {
             if (idle_spawn_rate >= 8) {
-                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf,
+                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf, APLOGNO(00221)
                     "server seems busy, (you may need "
                     "to increase StartServers, or Min/MaxSpareServers), "
                     "spawning %d children, there are %d idle, and "
                     "%d total children", idle_spawn_rate,
                     idle_count, total_non_dead);
             }
