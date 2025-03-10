                      wd_server_conf->parent_workers);
     }
     if ((wl = ap_list_provider_names(pconf, AP_WATCHDOG_PGROUP,
                                             AP_WATCHDOG_CVERSION))) {
         const ap_list_provider_names_t *wn;
         int i;
+        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(02977)
+                "Watchdog: found child providers.");
 
         wn = (ap_list_provider_names_t *)wl->elts;
         for (i = 0; i < wl->nelts; i++) {
             ap_watchdog_t *w = ap_lookup_provider(AP_WATCHDOG_PGROUP,
                                                   wn[i].provider_name,
                                                   AP_WATCHDOG_CVERSION);
+            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(02978)
+                    "Watchdog: Looking for child (%s).", wn[i].provider_name);
             if (w) {
                 if (!w->active) {
                     int status = ap_run_watchdog_need(s, w->name, 0,
                                                       w->singleton);
                     if (status == OK) {
                         /* One of the modules returned OK to this watchog.
