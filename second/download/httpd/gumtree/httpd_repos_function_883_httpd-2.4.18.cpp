static int ssl_ctx_compatible(modssl_ctx_t *ctx1, 
                              modssl_ctx_t *ctx2) 
{
    if (!ctx1 || !ctx2 
        || (ctx1->protocol != ctx2->protocol)
        || !ssl_auth_compatible(&ctx1->auth, &ctx2->auth)
        || !ssl_pk_server_compatible(ctx1->pks, ctx2->pks)) {
        return 0;
    }
    return 1;
}