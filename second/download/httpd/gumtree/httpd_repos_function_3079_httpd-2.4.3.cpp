static const char *set_error_document(cmd_parms *cmd, void *conf_,
                                      const char *errno_str, const char *msg)
{
    core_dir_config *conf = conf_;
    int error_number, index_number, idx500;
    enum { MSG, LOCAL_PATH, REMOTE_PATH } what = MSG;

    /* 1st parameter should be a 3 digit number, which we recognize;
     * convert it into an array index
     */
    error_number = atoi(errno_str);
    idx500 = ap_index_of_response(HTTP_INTERNAL_SERVER_ERROR);

    if (error_number == HTTP_INTERNAL_SERVER_ERROR) {
        index_number = idx500;
    }
    else if ((index_number = ap_index_of_response(error_number)) == idx500) {
        return apr_pstrcat(cmd->pool, "Unsupported HTTP response code ",
                           errno_str, NULL);
    }

    /* Heuristic to determine second argument. */
    if (ap_strchr_c(msg,' '))
        what = MSG;
    else if (msg[0] == '/')
        what = LOCAL_PATH;
    else if (ap_is_url(msg))
        what = REMOTE_PATH;
    else
        what = MSG;

    /* The entry should be ignored if it is a full URL for a 401 error */

    if (error_number == 401 && what == REMOTE_PATH) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, cmd->server, APLOGNO(00113)
                     "cannot use a full URL in a 401 ErrorDocument "
                     "directive --- ignoring!");
    }
    else { /* Store it... */
        if (conf->response_code_strings == NULL) {
            conf->response_code_strings =
                apr_pcalloc(cmd->pool,
                            sizeof(*conf->response_code_strings) *
                            RESPONSE_CODES);
        }

        if (strcmp(msg, "default") == 0) {
            /* special case: ErrorDocument 404 default restores the
             * canned server error response
             */
            conf->response_code_strings[index_number] = &errordocument_default;
        }
        else {
            /* hack. Prefix a " if it is a msg; as that is what
             * http_protocol.c relies on to distinguish between
             * a msg and a (local) path.
             */
            conf->response_code_strings[index_number] = (what == MSG) ?
                    apr_pstrcat(cmd->pool, "\"", msg, NULL) :
                    apr_pstrdup(cmd->pool, msg);
        }
    }

    return NULL;
}