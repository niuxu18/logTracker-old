static X509 *stapling_get_issuer(modssl_ctx_t *mctx, X509 *x)
{
    X509 *issuer = NULL;
    int i;
    X509_STORE *st = SSL_CTX_get_cert_store(mctx->ssl_ctx);
    X509_STORE_CTX *inctx;
    STACK_OF(X509) *extra_certs = NULL;

#ifdef OPENSSL_NO_SSL_INTERN
    SSL_CTX_get_extra_chain_certs(mctx->ssl_ctx, &extra_certs);
#else
    extra_certs = mctx->ssl_ctx->extra_certs;
#endif

    for (i = 0; i < sk_X509_num(extra_certs); i++) {
        issuer = sk_X509_value(extra_certs, i);
        if (X509_check_issued(issuer, x) == X509_V_OK) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            CRYPTO_add(&issuer->references, 1, CRYPTO_LOCK_X509);
#else
            X509_up_ref(issuer);
#endif
            return issuer;
        }
    }

    inctx = X509_STORE_CTX_new();
    if (!X509_STORE_CTX_init(inctx, st, NULL, NULL))
        return 0;
    if (X509_STORE_CTX_get1_issuer(&issuer, inctx, x) <= 0)
        issuer = NULL;
    X509_STORE_CTX_cleanup(inctx);
    X509_STORE_CTX_free(inctx);
    return issuer;
}