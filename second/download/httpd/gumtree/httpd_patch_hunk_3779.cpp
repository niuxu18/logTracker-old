                     return decl_die(access_status, "check user", r);
                 }
                 if ((access_status = ap_run_auth_checker(r)) != OK) {
                     return decl_die(access_status, "check authorization", r);
                 }
             }
+            else if (access_status == OK) {
+                ap_log_rerror(APLOG_MARK, APLOG_TRACE3, 0, r,
+                              "request authorized without authentication by "
+                              "access_checker_ex hook: %s", r->uri);
+            }
+            else {
+                return decl_die(access_status, "check access", r);
+            }
             break;
         }
     }
     /* XXX Must make certain the ap_run_type_checker short circuits mime
      * in mod-proxy for r->proxyreq && r->parsed_uri.scheme
      *                              && !strcmp(r->parsed_uri.scheme, "http")
