static void ssl_session_log(server_rec *s,
                            const char *request,
                            IDCONST unsigned char *id,
                            unsigned int idlen,
                            const char *status,
                            const char *result,
                            long timeout)
{
    char buf[MODSSL_SESSION_ID_STRING_LEN];
    char timeout_str[56] = {'\0'};

    if (!APLOGdebug(s)) {
        return;
    }

    if (timeout) {
        apr_snprintf(timeout_str, sizeof(timeout_str),
                     "timeout=%lds ", timeout);
    }

    ap_log_error(APLOG_MARK, APLOG_TRACE2, 0, s,
                 "Inter-Process Session Cache: "
                 "request=%s status=%s id=%s %s(session %s)",
                 request, status,
                 modssl_SSL_SESSION_id2sz(id, idlen, buf, sizeof(buf)),
                 timeout_str, result);
}