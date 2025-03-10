static authz_status user_check_authorization(request_rec *r,
                                             const char *require_args,
                                             const void *parsed_require_args)
{
    const char *t, *w;

    if (!r->user) {
        return AUTHZ_DENIED_NO_USER;
    }

    t = require_args;
    while ((w = ap_getword_conf(r->pool, &t)) && w[0]) {
        if (!strcmp(r->user, w)) {
            return AUTHZ_GRANTED;
        }
    }

    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01663)
                  "access to %s failed, reason: user '%s' does not meet "
                  "'require'ments for user to be allowed access",
                  r->uri, r->user);

    return AUTHZ_DENIED;
}