void ssl_io_filter_init(conn_rec *c, request_rec *r, SSL *ssl)
{
    ssl_filter_ctx_t *filter_ctx;

    filter_ctx = apr_palloc(c->pool, sizeof(ssl_filter_ctx_t));

    filter_ctx->config          = myConnConfig(c);

    ap_add_output_filter(ssl_io_coalesce, NULL, r, c);

    filter_ctx->pOutputFilter   = ap_add_output_filter(ssl_io_filter,
                                                       filter_ctx, r, c);

    filter_ctx->pbioWrite       = BIO_new(&bio_filter_out_method);
    filter_ctx->pbioWrite->ptr  = (void *)bio_filter_out_ctx_new(filter_ctx, c);

    /* We insert a clogging input filter. Let the core know. */
    c->clogging_input_filters = 1;

    ssl_io_input_add_filter(filter_ctx, c, r, ssl);

    SSL_set_bio(ssl, filter_ctx->pbioRead, filter_ctx->pbioWrite);
    filter_ctx->pssl            = ssl;

    apr_pool_cleanup_register(c->pool, (void*)filter_ctx,
                              ssl_io_filter_cleanup, apr_pool_cleanup_null);

    if (APLOG_CS_IS_LEVEL(c, mySrvFromConn(c), APLOG_TRACE4)) {
        BIO_set_callback(SSL_get_rbio(ssl), ssl_io_data_cb);
        BIO_set_callback_arg(SSL_get_rbio(ssl), (void *)ssl);
    }

    return;
}