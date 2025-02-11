int ssl_callback_ServerNameIndication(SSL *ssl, int *al, modssl_ctx_t *mctx)
{
    conn_rec *c = (conn_rec *)SSL_get_app_data(ssl);
    apr_status_t status = init_vhost(c, ssl);
    
    return (status == APR_SUCCESS)? SSL_TLSEXT_ERR_OK : SSL_TLSEXT_ERR_NOACK;
}