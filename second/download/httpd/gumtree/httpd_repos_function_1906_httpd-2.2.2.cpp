static void register_hooks(apr_pool_t *p)
{
    static const char * const aszPre[]={ "mod_authz_owner.c", NULL };

    ap_hook_auth_checker(check_user_access, aszPre, NULL, APR_HOOK_MIDDLE);
}