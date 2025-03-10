       netrcfile = override;
       netrc_alloc = TRUE;
     }
   }
 #endif /* CURLDEBUG */
   if(!netrcfile) {
+    home = curl_getenv("HOME"); /* portable environment reader */
+    if(home) {
+      home_alloc = TRUE;
 #if defined(HAVE_GETPWUID) && defined(HAVE_GETEUID)
-    struct passwd *pw;
-    pw= getpwuid(geteuid());
-    if (pw) {
+    }
+    else {
+      struct passwd *pw;
+      pw= getpwuid(geteuid());
+      if (pw) {
 #ifdef	VMS
-      home = decc$translate_vms(pw->pw_dir);
+        home = decc$translate_vms(pw->pw_dir);
 #else
-      home = pw->pw_dir;
+        home = pw->pw_dir;
 #endif
-    }
+      }
 #endif
-  
-    if(!home) {
-      home = curl_getenv("HOME"); /* portable environment reader */
-      if(!home)
-        return -1;
-      home_alloc = TRUE;
     }
 
+    if(!home)
+      return -1;
+
     netrcfile = curl_maprintf("%s%s%s", home, DIR_CHAR, NETRC);
     if(!netrcfile) {
       if(home_alloc)
         free(home);
       return -1;
     }
