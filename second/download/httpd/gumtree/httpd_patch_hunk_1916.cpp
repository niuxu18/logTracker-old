 AP_IMPLEMENT_HOOK_RUN_FIRST(int,auth_checker,
                             (request_rec *r), (r), DECLINED)
 AP_IMPLEMENT_HOOK_VOID(insert_filter, (request_rec *r), (r))
 AP_IMPLEMENT_HOOK_RUN_ALL(int, create_request,
                           (request_rec *r), (r), OK, DECLINED)
 
+static int auth_internal_per_conf = 0;
+static int auth_internal_per_conf_hooks = 0;
+static int auth_internal_per_conf_providers = 0;
+
 
-static int decl_die(int status, char *phase, request_rec *r)
+static int decl_die(int status, const char *phase, request_rec *r)
 {
     if (status == DECLINED) {
         ap_log_rerror(APLOG_MARK, APLOG_CRIT, 0, r,
                       "configuration error:  couldn't %s: %s", phase, r->uri);
         return HTTP_INTERNAL_SERVER_ERROR;
     }
