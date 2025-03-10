     if (s->dynamicCertMemCacheSize != std::numeric_limits<size_t>::max())
         storeAppendPrintf(e, "dynamic_cert_mem_cache_size=%lu%s\n", (unsigned long)s->dynamicCertMemCacheSize, B_BYTES_STR);
 #endif
 }
 
 static void
-dump_PortCfg(StoreEntry * e, const char *n, const AnyP::PortCfg * s)
+dump_PortCfg(StoreEntry * e, const char *n, const AnyP::PortCfgPointer &s)
 {
-    while (s) {
-        dump_generic_port(e, n, s);
+    for (AnyP::PortCfgPointer p = s; p != NULL; p = p->next) {
+        dump_generic_port(e, n, p);
         storeAppendPrintf(e, "\n");
-        s = s->next;
-    }
-}
-
-static void
-free_PortCfg(AnyP::PortCfg ** head)
-{
-    AnyP::PortCfg *s;
-
-    while ((s = *head) != NULL) {
-        *head = s->next;
-        cbdataReferenceDone(s);
     }
 }
 
 void
 configFreeMemory(void)
 {
     free_all();
-#if USE_SSL
+#if USE_OPENSSL
     SSL_CTX_free(Config.ssl_client.sslContext);
+    Ssl::unloadSquidUntrusted();
 #endif
 }
 
 void
 requirePathnameExists(const char *name, const char *path)
 {
