{
    apr_file_t *f = NULL;
    apr_finfo_t finfo;
    char time_str[APR_CTIME_LEN];
    int log_flags = rv ? APLOG_ERR : APLOG_ERR;

    /* Intentional no APLOGNO */
    /* Callee provides APLOGNO in error text */
    ap_log_rerror(APLOG_MARK, log_flags, rv, r,
                  "%s%s: %s", logno ? logno : "", error, r->filename);

    /* XXX Very expensive mainline case! Open, then getfileinfo! */
    if (!conf->logname ||
        ((apr_stat(&finfo, conf->logname,
