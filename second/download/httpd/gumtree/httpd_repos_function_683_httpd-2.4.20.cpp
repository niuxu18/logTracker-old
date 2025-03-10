static void ssl_init_ctx_callbacks(server_rec *s,
                                   apr_pool_t *p,
                                   apr_pool_t *ptemp,
                                   modssl_ctx_t *mctx)
{
    SSL_CTX *ctx = mctx->ssl_ctx;

    SSL_CTX_set_tmp_dh_callback(ctx,  ssl_callback_TmpDH);

    SSL_CTX_set_info_callback(ctx, ssl_callback_Info);

#ifdef HAVE_TLS_ALPN
    SSL_CTX_set_alpn_select_cb(ctx, ssl_callback_alpn_select, NULL);
#endif
}