     trunc = 0;
     for(; (len > 0) && ((s[len-1] == ' ') || (s[len-1] == '\0')); len--)
         trunc++;
     rows = (len / DUMP_WIDTH);
     if ((rows * DUMP_WIDTH) < len)
         rows++;
-    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, srvr,
+    ap_log_error(APLOG_MARK, APLOG_TRACE7, 0, srvr,
             "+-------------------------------------------------------------------------+");
     for(i = 0 ; i< rows; i++) {
 #if APR_CHARSET_EBCDIC
         char ebcdic_text[DUMP_WIDTH];
-        /* Determine how many bytes we are going to process in this row. */
         j = DUMP_WIDTH;
         if ((i * DUMP_WIDTH + j) > len)
             j = len % DUMP_WIDTH;
-        if (j == 0) j = DUMP_WIDTH;
-        memcpy(ebcdic_text, (char *)(s) + i * DUMP_WIDTH, j);
+        if (j == 0)
+            j = DUMP_WIDTH;
+        memcpy(ebcdic_text,(char *)(s) + i * DUMP_WIDTH, j);
         ap_xlate_proto_from_ascii(ebcdic_text, j);
-#endif
+#endif /* APR_CHARSET_EBCDIC */
         apr_snprintf(tmp, sizeof(tmp), "| %04x: ", i * DUMP_WIDTH);
         apr_cpystrn(buf, tmp, sizeof(buf));
         for (j = 0; j < DUMP_WIDTH; j++) {
             if (((i * DUMP_WIDTH) + j) >= len)
                 apr_cpystrn(buf+strlen(buf), "   ", sizeof(buf)-strlen(buf));
             else {
