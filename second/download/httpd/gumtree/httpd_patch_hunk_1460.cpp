 /* display copyright information */
 static void copyright(void)
 {
     if (!use_html) {
         printf("This is ApacheBench, Version %s\n", AP_AB_BASEREVISION " <$Revision: 1.146 $> apache-2.0");
         printf("Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/\n");
-        printf("Copyright 1997-2005 The Apache Software Foundation, http://www.apache.org/\n");
+        printf("Copyright 2006 The Apache Software Foundation, http://www.apache.org/\n");
         printf("\n");
     }
     else {
         printf("<p>\n");
         printf(" This is ApacheBench, Version %s <i>&lt;%s&gt;</i> apache-2.0<br>\n", AP_AB_BASEREVISION, "$Revision: 1.146 $");
         printf(" Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/<br>\n");
-        printf(" Copyright 1997-2005 The Apache Software Foundation, http://www.apache.org/<br>\n");
+        printf(" Copyright 2006 The Apache Software Foundation, http://www.apache.org/<br>\n");
         printf("</p>\n<p>\n");
     }
 }
 
 /* display usage information */
 static void usage(const char *progname)
