         extract_dn(t, nids, "SSL_CLIENT_S_DN_", X509_get_subject_name(xs), p);
         extract_dn(t, nids, "SSL_CLIENT_I_DN_", X509_get_issuer_name(xs), p);
         X509_free(xs);
     }
 }
 
-const char *ssl_ext_lookup(apr_pool_t *p, conn_rec *c, int peer,
-                           const char *oidnum)
+/* For an extension type which OpenSSL does not recognize, attempt to
+ * parse the extension type as a primitive string.  This will fail for
+ * any structured extension type per the docs.  Returns non-zero on
+ * success and writes the string to the given bio. */
+static int dump_extn_value(BIO *bio, ASN1_OCTET_STRING *str)
+{
+    unsigned char *pp = str->data;
+    ASN1_STRING *ret = ASN1_STRING_new();
+    int rv = 0;
+    
+    /* This allows UTF8String, IA5String, VisibleString, or BMPString;
+     * conversion to UTF-8 is forced. */
+    if (d2i_DISPLAYTEXT(&ret, &pp, str->length)) {
+        ASN1_STRING_print_ex(bio, ret, ASN1_STRFLGS_UTF8_CONVERT);
+        rv = 1;
+    }
+
+    ASN1_STRING_free(ret);
+    return rv;
+}
+
+apr_array_header_t *ssl_ext_list(apr_pool_t *p, conn_rec *c, int peer,
+                                 const char *extension)
 {
     SSLConnRec *sslconn = myConnConfig(c);
-    SSL *ssl;
+    SSL *ssl = NULL;
+    apr_array_header_t *array = NULL;
     X509 *xs = NULL;
-    ASN1_OBJECT *oid;
+    ASN1_OBJECT *oid = NULL;
     int count = 0, j;
-    char *result = NULL;
 
-    if (!sslconn || !sslconn->ssl) {
+    if (!sslconn || !sslconn->ssl || !extension) {
         return NULL;
     }
     ssl = sslconn->ssl;
 
-    oid = OBJ_txt2obj(oidnum, 1);
+    /* We accept the "extension" string to be converted as
+     * a long name (nsComment), short name (DN) or
+     * numeric OID (1.2.3.4).
+     */
+    oid = OBJ_txt2obj(extension, 0);
     if (!oid) {
+        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
+                      "could not parse OID '%s'", extension);
         ERR_clear_error();
         return NULL;
     }
 
     xs = peer ? SSL_get_peer_certificate(ssl) : SSL_get_certificate(ssl);
     if (xs == NULL) {
         return NULL;
     }
 
     count = X509_get_ext_count(xs);
-
+    /* Create an array large enough to accomodate every extension. This is
+     * likely overkill, but safe.
+     */
+    array = apr_array_make(p, count, sizeof(char *));
     for (j = 0; j < count; j++) {
         X509_EXTENSION *ext = X509_get_ext(xs, j);
 
         if (OBJ_cmp(ext->object, oid) == 0) {
             BIO *bio = BIO_new(BIO_s_mem());
 
-            if (X509V3_EXT_print(bio, ext, 0, 0) == 1) {
+            /* We want to obtain a string representation of the extensions
+             * value and add it to the array we're building.
+             * X509V3_EXT_print() doesn't know about all the possible
+             * data types, but the value is stored as an ASN1_OCTET_STRING
+             * allowing us a fallback in case of X509V3_EXT_print
+             * not knowing how to handle the data.
+             */
+            if (X509V3_EXT_print(bio, ext, 0, 0) == 1 ||
+                dump_extn_value(bio, X509_EXTENSION_get_data(ext)) == 1) {
                 BUF_MEM *buf;
-
+                char **ptr = apr_array_push(array);
                 BIO_get_mem_ptr(bio, &buf);
-                result = apr_pstrmemdup(p, buf->data, buf->length);
+                *ptr = apr_pstrmemdup(p, buf->data, buf->length);
+            } else {
+                ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, c,
+                              "Found an extension '%s', but failed to "
+                              "create a string from it", extension);
             }
-
             BIO_vfree(bio);
-            break;
         }
     }
 
+    if (array->nelts == 0)
+        array = NULL;
+
     if (peer) {
         /* only SSL_get_peer_certificate raises the refcount */
         X509_free(xs);
     }
 
+    ASN1_OBJECT_free(oid);
     ERR_clear_error();
-    return result;
+    return array;
 }
 
 static char *ssl_var_lookup_ssl_compress_meth(SSL *ssl)
 {
     char *result = "NULL";
 #ifdef OPENSSL_VERSION_NUMBER
