             /* XXX Two significant problems; 1, we don't check to see if we are
              * setting redundant filters.    2, we insert these in the types config
              * hook, which may be too early (dunno.)
              */
             if (exinfo->input_filters && r->proxyreq == PROXYREQ_NONE) {
                 const char *filter, *filters = exinfo->input_filters;
-                while (*filters 
+                while (*filters
                     && (filter = ap_getword(r->pool, &filters, ';'))) {
                     ap_add_input_filter(filter, NULL, r, r->connection);
                 }
                 if (conf->multimatch & MULTIMATCH_FILTERS) {
                     found = 1;
                 }
             }
             if (exinfo->output_filters && r->proxyreq == PROXYREQ_NONE) {
                 const char *filter, *filters = exinfo->output_filters;
-                while (*filters 
+                while (*filters
                     && (filter = ap_getword(r->pool, &filters, ';'))) {
                     ap_add_output_filter(filter, NULL, r, r->connection);
                 }
                 if (conf->multimatch & MULTIMATCH_FILTERS) {
                     found = 1;
                 }
