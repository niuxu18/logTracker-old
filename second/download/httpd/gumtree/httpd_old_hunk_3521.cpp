    v.name = name;

    apr_table_do((int (*) (void *, const char *, const char *))
                 extract_cookie_line, (void *) &v, r->headers_in,
                 "Cookie", "Cookie2", NULL);
    if (v.duplicated) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, LOG_PREFIX
         "client submitted cookie '%s' more than once: %s", v.name, r->uri);
        return APR_EGENERAL;
    }

    /* remove our cookie(s), and replace them */
    if (remove) {
