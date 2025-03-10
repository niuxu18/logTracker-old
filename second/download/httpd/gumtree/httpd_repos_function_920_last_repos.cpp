int ssl_callback_SRPServerParams(SSL *ssl, int *ad, void *arg)
{
    modssl_ctx_t *mctx = (modssl_ctx_t *)arg;
    char *username = SSL_get_srp_username(ssl);
    SRP_user_pwd *u;

    if (username == NULL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        || (u = SRP_VBASE_get_by_user(mctx->srp_vbase, username)) == NULL) {
#else
        || (u = SRP_VBASE_get1_by_user(mctx->srp_vbase, username)) == NULL) {
#endif
        *ad = SSL_AD_UNKNOWN_PSK_IDENTITY;
        return SSL3_AL_FATAL;
    }

    if (SSL_set_srp_server_param(ssl, u->N, u->g, u->s, u->v, u->info) < 0) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        SRP_user_pwd_free(u);
#endif
        *ad = SSL_AD_INTERNAL_ERROR;
        return SSL3_AL_FATAL;
    }

    /* reset all other options */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    SRP_user_pwd_free(u);
#endif
    SSL_set_verify(ssl, SSL_VERIFY_NONE,  ssl_callback_SSLVerify);
    return SSL_ERROR_NONE;
}