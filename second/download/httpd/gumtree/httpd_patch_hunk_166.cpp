             }
             else {
                 ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                             "unknown parameter \"%s\" to tag include in %s",
                             tag, r->filename);
                 CREATE_ERROR_BUCKET(ctx, tmp_buck, head_ptr, *inserted_head);
+                return 1;
             }
         }
     }
     return 0;
 }
 
