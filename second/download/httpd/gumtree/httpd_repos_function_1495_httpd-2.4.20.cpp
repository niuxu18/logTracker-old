static const char *add_redirect_internal(cmd_parms *cmd,
                                         alias_dir_conf *dirconf,
                                         const char *arg1, const char *arg2,
                                         const char *arg3, int use_regex)
{
    alias_entry *new;
    server_rec *s = cmd->server;
    alias_server_conf *serverconf = ap_get_module_config(s->module_config,
                                                         &alias_module);
    int status = (int) (long) cmd->info;
    int grokarg1 = 1;
    ap_regex_t *regex = NULL;
    const char *fake = arg2;
    const char *url = arg3;

    /*
     * Logic flow:
     *   Go ahead and try to grok the 1st arg, in case it is a
     *   Redirect status. Now if we have 3 args, we expect that
     *   we were able to understand that 1st argument (it's something
     *   we expected, so if not, then we bail
     */
    if (!strcasecmp(arg1, "permanent"))
        status = HTTP_MOVED_PERMANENTLY;
    else if (!strcasecmp(arg1, "temp"))
        status = HTTP_MOVED_TEMPORARILY;
    else if (!strcasecmp(arg1, "seeother"))
        status = HTTP_SEE_OTHER;
    else if (!strcasecmp(arg1, "gone")) {
        status = HTTP_GONE;
        grokarg1 = -1;
    }
    else if (apr_isdigit(*arg1)) {
        status = atoi(arg1);
        if (!ap_is_HTTP_REDIRECT(status)) {
            grokarg1 = -1;
        }
    }
    else {
        grokarg1 = 0;
    }

    if (arg3 && !grokarg1)
        return "Redirect: invalid first argument (of three)";

    /*
     * if we have the 2nd arg and we understand the 1st one as a redirect
     * status (3xx, but not things like 404 /robots.txt), or if we have the
     * 1st arg but don't understand it, we use the expression syntax assuming
     * a path from the location.
     *
     * if we understand the first arg but have no second arg, we are dealing
     * with a status like "GONE" or a non-redirect status (e.g. 404, 503).
     */
    if (!cmd->path) {
        /* <Location> context only for now */
        ;
    }
    else if ((grokarg1 > 0 && arg2 && !arg3) || (!grokarg1 && !arg2)) {
        const char *expr_err = NULL;

        url = grokarg1 ? arg2 : arg1;
        dirconf->redirect =
                ap_expr_parse_cmd(cmd, url, AP_EXPR_FLAG_STRING_RESULT,
                        &expr_err, NULL);
        if (expr_err) {
            return apr_pstrcat(cmd->temp_pool,
                    "Cannot parse redirect expression '", url, "': ", expr_err,
                    NULL);
        }

        dirconf->redirect_status = status;
        dirconf->redirect_set = 1;

        return NULL;

    }
    else if (grokarg1 < 0 && !arg2) {

        dirconf->redirect_status = status;
        dirconf->redirect_set = 1;

        return NULL;

    }

    /*
     * if we don't have the 3rd arg and we didn't understand the 1st
     * one, then assume URL-path URL. This also handles case, eg, GONE
     * we even though we don't have a 3rd arg, we did understand the 1st
     * one, so we don't want to re-arrange
     */
    if (!arg3 && !grokarg1) {
        fake = arg1;
        url = arg2;
    }

    if (use_regex) {
        regex = ap_pregcomp(cmd->pool, fake, AP_REG_EXTENDED);
        if (regex == NULL)
            return "Regular expression could not be compiled.";
    }

    if (ap_is_HTTP_REDIRECT(status)) {
        if (!url)
            return "URL to redirect to is missing";
        /* PR#35314: we can allow path components here;
         * they get correctly resolved to full URLs.
         */
        if (!use_regex && !ap_is_url(url) && (url[0] != '/'))
            return "Redirect to non-URL";
    }
    else {
        if (url)
            return "Redirect URL not valid for this status";
    }

    if (cmd->path)
        new = apr_array_push(dirconf->redirects);
    else
        new = apr_array_push(serverconf->redirects);

    new->fake = fake;
    new->real = url;
    new->regexp = regex;
    new->redir_status = status;
    return NULL;
}