static int proxy_post_config(apr_pool_t *pconf, apr_pool_t *plog,
                             apr_pool_t *ptemp, server_rec *s)
{
    proxy_ssl_enable = APR_RETRIEVE_OPTIONAL_FN(ssl_proxy_enable);
    proxy_ssl_disable = APR_RETRIEVE_OPTIONAL_FN(ssl_engine_disable);

    return OK;
}