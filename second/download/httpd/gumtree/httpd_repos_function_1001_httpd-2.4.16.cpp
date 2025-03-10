static int lua_map_handler_fixups(request_rec *r)
{
    /* If there is no handler set yet, this might be a LuaMapHandler request */
    if (r->handler == NULL) {
        int n = 0;
        ap_regmatch_t match[10];
        const ap_lua_dir_cfg *cfg = ap_get_module_config(r->per_dir_config,
                                                     &lua_module);
        for (n = 0; n < cfg->mapped_handlers->nelts; n++) {
            ap_lua_mapped_handler_spec *hook_spec =
            ((ap_lua_mapped_handler_spec **) cfg->mapped_handlers->elts)[n];

            if (hook_spec == NULL) {
                continue;
            }
            if (!ap_regexec(hook_spec->uri_pattern, r->uri, 10, match, 0)) {
                r->handler = apr_pstrdup(r->pool, "lua-map-handler");
                return OK;
            }
        }
    }
    return DECLINED;
}