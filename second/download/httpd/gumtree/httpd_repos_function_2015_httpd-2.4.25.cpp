static void ap_proxy_balancer_register_hook(apr_pool_t *p)
{
    /* Only the mpm_winnt has child init hook handler.
     * make sure that we are called after the mpm
     * initializes
     */
    static const char *const aszPred[] = { "mpm_winnt.c", "mod_slotmem_shm.c", NULL};
    static const char *const aszPred2[] = { "mod_proxy.c", NULL};
     /* manager handler */
    ap_hook_post_config(balancer_post_config, aszPred2, NULL, APR_HOOK_MIDDLE);
    ap_hook_pre_config(balancer_pre_config, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_handler(balancer_handler, NULL, NULL, APR_HOOK_FIRST);
    ap_hook_child_init(balancer_child_init, aszPred, NULL, APR_HOOK_MIDDLE);
    proxy_hook_pre_request(proxy_balancer_pre_request, NULL, NULL, APR_HOOK_FIRST);
    proxy_hook_post_request(proxy_balancer_post_request, NULL, NULL, APR_HOOK_FIRST);
    proxy_hook_canon_handler(proxy_balancer_canon, NULL, NULL, APR_HOOK_FIRST);
}