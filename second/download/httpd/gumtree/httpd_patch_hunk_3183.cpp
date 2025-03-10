     }
     else {
         depth = mctx->auth.verify_depth;
     }
 
     if (errdepth > depth) {
-        ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, conn,
+        ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, conn, APLOGNO(02040)
                       "Certificate Verification: Certificate Chain too long "
                       "(chain has %d certificates, but maximum allowed are "
                       "only %d)",
                       errdepth, depth);
 
         errnum = X509_V_ERR_CERT_CHAIN_TOO_LONG;
