                     if (d->icon_width) {
                         ap_rprintf(r, " width=\"%d\"", d->icon_width);
                     }
                     if (d->icon_height) {
                         ap_rprintf(r, " height=\"%d\"", d->icon_height);
                     }
-                    ap_rputs(" />", r);
+
+                    if (autoindex_opts & EMIT_XHTML) {
+                        ap_rputs(" /", r);
+                    }
+                    ap_rputs(">", r);
                 }
                 else {
                     ap_rputs("     ", r);
                 }
                 if (autoindex_opts & ICONS_ARE_LINKS) {
                     ap_rputs("</a> ", r);
