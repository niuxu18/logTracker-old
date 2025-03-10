         mctx->pks->certs[SSL_AIDX_DSA]
 #ifndef OPENSSL_NO_EC
       || mctx->pks->certs[SSL_AIDX_ECC]
 #endif
         )
     {
-        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s,
+        ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(01892)
                 "Illegal attempt to re-initialise SSL for server "
-                "(theoretically shouldn't happen!)");
+                "(SSLEngine On should go in the VirtualHost, not in global scope.)");
         ssl_die();
     }
 }
 
 #ifndef OPENSSL_NO_TLSEXT
 static void ssl_init_ctx_tls_extensions(server_rec *s,
