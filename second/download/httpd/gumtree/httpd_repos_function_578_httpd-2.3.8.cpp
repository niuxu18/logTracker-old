static APR_INLINE const char *header_inout_cmd(cmd_parms *cmd,
                                               void *indirconf,
                                               const char *action,
                                               const char *hdr,
                                               const char *value,
                                               const char *subs,
                                               const char *envclause)
{
    headers_conf *dirconf = indirconf;
    const char *condition_var = NULL;
    const char *colon;
    header_entry *new;
    ap_parse_node_t *expr = NULL;

    apr_array_header_t *fixup = (cmd->info == &hdr_in)
        ? dirconf->fixup_in   : (cmd->info == &hdr_err)
        ? dirconf->fixup_err
        : dirconf->fixup_out;

    new = (header_entry *) apr_array_push(fixup);

    if (!strcasecmp(action, "set"))
        new->action = hdr_set;
    else if (!strcasecmp(action, "add"))
        new->action = hdr_add;
    else if (!strcasecmp(action, "append"))
        new->action = hdr_append;
    else if (!strcasecmp(action, "merge"))
        new->action = hdr_merge;
    else if (!strcasecmp(action, "unset"))
        new->action = hdr_unset;
    else if (!strcasecmp(action, "echo"))
        new->action = hdr_echo;
    else if (!strcasecmp(action, "edit"))
        new->action = hdr_edit;
    else if (!strcasecmp(action, "edit*"))
        new->action = hdr_edit_r;
    else
        return "first argument must be 'add', 'set', 'append', 'merge', "
               "'unset', 'echo', 'edit', or 'edit*'.";

    if (new->action == hdr_edit || new->action == hdr_edit_r) {
        if (subs == NULL) {
            return "Header edit requires a match and a substitution";
        }
        new->regex = ap_pregcomp(cmd->pool, value, AP_REG_EXTENDED);
        if (new->regex == NULL) {
            return "Header edit regex could not be compiled";
        }
        new->subs = subs;
    }
    else {
        /* there's no subs, so envclause is really that argument */
        if (envclause != NULL) {
            return "Too many arguments to directive";
        }
        envclause = subs;
    }
    if (new->action == hdr_unset) {
        if (value) {
            if (envclause) {
                return "header unset takes two arguments";
            }
            envclause = value;
            value = NULL;
        }
    }
    else if (new->action == hdr_echo) {
        ap_regex_t *regex;

        if (value) {
            if (envclause) {
                return "Header echo takes two arguments";
            }
            envclause = value;
            value = NULL;
        }
        if (cmd->info != &hdr_out && cmd->info != &hdr_err)
            return "Header echo only valid on Header "
                   "directives";
        else {
            regex = ap_pregcomp(cmd->pool, hdr, AP_REG_EXTENDED | AP_REG_NOSUB);
            if (regex == NULL) {
                return "Header echo regex could not be compiled";
            }
        }
        new->regex = regex;
    }
    else if (!value)
        return "Header requires three arguments";

    /* Handle the envclause on Header */
    if (envclause != NULL) {
        if (strcasecmp(envclause, "early") == 0) {
            condition_var = condition_early;
        }
        else if (strncasecmp(envclause, "env=", 4) == 0) {
            if ((envclause[4] == '\0')
                || ((envclause[4] == '!') && (envclause[5] == '\0'))) {
                return "error: missing environment variable name. "
                    "envclause should be in the form env=envar ";
            }
            condition_var = envclause + 4;
        }
        else {
            int err = 0;
            expr = ap_expr_parse(cmd->pool, envclause, &err);
            if (err) {
                return "Can't parse envclause/expression";
            }
        }
    }

    if ((colon = ap_strchr_c(hdr, ':'))) {
        hdr = apr_pstrmemdup(cmd->pool, hdr, colon-hdr);
    }

    new->header = hdr;
    new->condition_var = condition_var;
    new->expr = expr;

    return parse_format_string(cmd->pool, new, value);
}