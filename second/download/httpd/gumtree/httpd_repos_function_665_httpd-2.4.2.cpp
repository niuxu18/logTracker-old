static void ssl_init_ctx_protocol(server_rec *s,
                                  apr_pool_t *p,
                                  apr_pool_t *ptemp,
                                  modssl_ctx_t *mctx)
{
    SSL_CTX *ctx = NULL;
    MODSSL_SSL_METHOD_CONST SSL_METHOD *method = NULL;
    char *cp;
    int protocol = mctx->protocol;
    SSLSrvConfigRec *sc = mySrvConfig(s);

    /*
     *  Create the new per-server SSL context
     */
    if (protocol == SSL_PROTOCOL_NONE) {
        ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(02231)
                "No SSL protocols available [hint: SSLProtocol]");
        ssl_die();
    }

    cp = apr_pstrcat(p,
                     (protocol & SSL_PROTOCOL_SSLV3 ? "SSLv3, " : ""),
                     (protocol & SSL_PROTOCOL_TLSV1 ? "TLSv1, " : ""),
#ifdef HAVE_TLSV1_X
                     (protocol & SSL_PROTOCOL_TLSV1_1 ? "TLSv1.1, " : ""),
                     (protocol & SSL_PROTOCOL_TLSV1_2 ? "TLSv1.2, " : ""),
#endif
                     NULL);
    cp[strlen(cp)-2] = NUL;

    ap_log_error(APLOG_MARK, APLOG_TRACE3, 0, s,
                 "Creating new SSL context (protocols: %s)", cp);

    if (protocol == SSL_PROTOCOL_SSLV3) {
        method = mctx->pkp ?
            SSLv3_client_method() : /* proxy */
            SSLv3_server_method();  /* server */
    }
    else if (protocol == SSL_PROTOCOL_TLSV1) {
        method = mctx->pkp ?
            TLSv1_client_method() : /* proxy */
            TLSv1_server_method();  /* server */
    }
#ifdef HAVE_TLSV1_X
    else if (protocol == SSL_PROTOCOL_TLSV1_1) {
        method = mctx->pkp ?
            TLSv1_1_client_method() : /* proxy */
            TLSv1_1_server_method();  /* server */
    }
    else if (protocol == SSL_PROTOCOL_TLSV1_2) {
        method = mctx->pkp ?
            TLSv1_2_client_method() : /* proxy */
            TLSv1_2_server_method();  /* server */
    }
#endif
    else { /* For multiple protocols, we need a flexible method */
        method = mctx->pkp ?
            SSLv23_client_method() : /* proxy */
            SSLv23_server_method();  /* server */
    }
    ctx = SSL_CTX_new(method);

    mctx->ssl_ctx = ctx;

    SSL_CTX_set_options(ctx, SSL_OP_ALL);

    /* always disable SSLv2, as per RFC 6176 */
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

    if (!(protocol & SSL_PROTOCOL_SSLV3)) {
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);
    }

    if (!(protocol & SSL_PROTOCOL_TLSV1)) {
        SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1);
    }

#ifdef HAVE_TLSV1_X
    if (!(protocol & SSL_PROTOCOL_TLSV1_1)) {
        SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_1);
    }

    if (!(protocol & SSL_PROTOCOL_TLSV1_2)) {
        SSL_CTX_set_options(ctx, SSL_OP_NO_TLSv1_2);
    }
#endif

#ifdef SSL_OP_CIPHER_SERVER_PREFERENCE
    if (sc->cipher_server_pref == TRUE) {
        SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
    }
#endif

#ifdef SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION
    if (sc->insecure_reneg == TRUE) {
        SSL_CTX_set_options(ctx, SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION);
    }
#endif

    SSL_CTX_set_app_data(ctx, s);

    /*
     * Configure additional context ingredients
     */
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);

#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
    /*
     * Disallow a session from being resumed during a renegotiation,
     * so that an acceptable cipher suite can be negotiated.
     */
    SSL_CTX_set_options(ctx, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif

#ifdef SSL_MODE_RELEASE_BUFFERS
    /* If httpd is configured to reduce mem usage, ask openssl to do so, too */
    if (ap_max_mem_free != APR_ALLOCATOR_MAX_FREE_UNLIMITED)
        SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
#endif
}