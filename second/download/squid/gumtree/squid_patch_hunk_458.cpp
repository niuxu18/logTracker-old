 
     for (r = storerepl_list; r && r->typestr; ++r) {
         if (strcmp(r->typestr, settings->type) == 0)
             return r->create(settings->args);
     }
 
-    debugs(20, 1, "ERROR: Unknown policy " << settings->type);
-    debugs(20, 1, "ERROR: Be sure to have set cache_replacement_policy");
-    debugs(20, 1, "ERROR:   and memory_replacement_policy in squid.conf!");
+    debugs(20, DBG_IMPORTANT, "ERROR: Unknown policy " << settings->type);
+    debugs(20, DBG_IMPORTANT, "ERROR: Be sure to have set cache_replacement_policy");
+    debugs(20, DBG_IMPORTANT, "ERROR:   and memory_replacement_policy in squid.conf!");
     fatalf("ERROR: Unknown policy %s\n", settings->type);
     return NULL;                /* NOTREACHED */
 }
 
 #if 0
 void
