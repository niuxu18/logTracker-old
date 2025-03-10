static const char *filesection(cmd_parms *cmd, void *mconfig, const char *arg)
{
    const char *errmsg;
    const char *endp = ap_strrchr_c(arg, '>');
    int old_overrides = cmd->override;
    char *old_path = cmd->path;
    core_dir_config *conf;
    regex_t *r = NULL;
    const command_rec *thiscmd = cmd->cmd;
    core_dir_config *c = mconfig;
    ap_conf_vector_t *new_file_conf = ap_create_per_dir_config(cmd->pool);
    const char *err = ap_check_cmd_context(cmd, NOT_IN_LIMIT|NOT_IN_LOCATION);

    if (err != NULL) {
        return err;
    }

    if (endp == NULL) {
        return unclosed_directive(cmd);
    }

    arg = apr_pstrndup(cmd->pool, arg, endp - arg);

    cmd->path = ap_getword_conf(cmd->pool, &arg);
    /* Only if not an .htaccess file */
    if (!old_path) {
        cmd->override = OR_ALL|ACCESS_CONF;
    }

    if (thiscmd->cmd_data) { /* <FilesMatch> */
        r = ap_pregcomp(cmd->pool, cmd->path, REG_EXTENDED|USE_ICASE);
        if (!r) {
            return "Regex could not be compiled";
        }
    }
    else if (!strcmp(cmd->path, "~")) {
        cmd->path = ap_getword_conf(cmd->pool, &arg);
        r = ap_pregcomp(cmd->pool, cmd->path, REG_EXTENDED|USE_ICASE);
        if (!r) {
            return "Regex could not be compiled";
        }
    }
    else {
        char *newpath;
        /* Ensure that the pathname is canonical, but we
         * can't test the case/aliases without a fixed path */
        if (apr_filepath_merge(&newpath, "", cmd->path,
                               0, cmd->pool) != APR_SUCCESS)
                return apr_pstrcat(cmd->pool, "<Files \"", cmd->path,
                               "\"> is invalid.", NULL);
        cmd->path = newpath;
    }

    /* initialize our config and fetch it */
    conf = ap_set_config_vectors(cmd->server, new_file_conf, cmd->path,
                                 &core_module, cmd->pool);

    errmsg = ap_walk_config(cmd->directive->first_child, cmd, new_file_conf);
    if (errmsg != NULL)
        return errmsg;

    conf->d = cmd->path;
    conf->d_is_fnmatch = apr_fnmatch_test(conf->d) != 0;
    conf->r = r;

    ap_add_file_conf(c, new_file_conf);

    if (*arg != '\0') {
        return apr_pstrcat(cmd->pool, "Multiple ", thiscmd->name,
                           "> arguments not (yet) supported.", NULL);
    }

    cmd->path = old_path;
    cmd->override = old_overrides;

    return NULL;
}