static const char *set_accf_map(cmd_parms *cmd, void *dummy,
                                const char *iproto, const char* iaccf)
{
    const char *err = ap_check_cmd_context(cmd, GLOBAL_ONLY);
    core_server_config *conf = ap_get_module_config(cmd->server->module_config,
                                                    &core_module);
    char* proto;
    char* accf;
    if (err != NULL) {
        return err;
    }

    proto = apr_pstrdup(cmd->pool, iproto);
    ap_str_tolower(proto);
    accf = apr_pstrdup(cmd->pool, iaccf);
    ap_str_tolower(accf);
    apr_table_set(conf->accf_map, proto, accf);

    return NULL;
}