static void register_hooks(apr_pool_t *p)
{
    ap_hook_post_read_request(remoteip_modify_connection, NULL, NULL, APR_HOOK_FIRST);
}