             */
            return DECLINED;
        }
    }
    else if (auth_result == AUTHZ_DENIED || auth_result == AUTHZ_NEUTRAL) {
        if (!after_authn || ap_auth_type(r) == NULL) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, APR_SUCCESS, r, APLOGNO(01630)
                          "client denied by server configuration: %s%s",
                          r->filename ? "" : "uri ",
                          r->filename ? r->filename : r->uri);

            return HTTP_FORBIDDEN;
        }
        else {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, APR_SUCCESS, r, APLOGNO(01631)
                          "user %s: authorization failure for \"%s\": ",
                          r->user, r->uri);

            if (conf->authz_forbidden_on_fail > 0) {
                return HTTP_FORBIDDEN;
            }
            else {
                /*
                 * If we're returning 401 to an authenticated user, tell them to
                 * try again. If unauthenticated, note_auth_failure has already
                 * been called during auth.
                 */
                if (r->user)
                    ap_note_auth_failure(r);
                return HTTP_UNAUTHORIZED;
            }
        }
    }
    else {
        /* We'll assume that the module has already said what its
         * error was in the logs.
         */
