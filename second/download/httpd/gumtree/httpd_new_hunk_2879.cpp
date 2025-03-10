
    retcode = apr_file_open(&f, metafilename, APR_READ, APR_OS_DEFAULT, r->pool);
    if (retcode != APR_SUCCESS) {
        if (APR_STATUS_IS_ENOENT(retcode)) {
            return DECLINED;
        }
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01562)
            "meta file permissions deny server access: %s", metafilename);
        return HTTP_FORBIDDEN;
    }

    /* read the headers in */
    rv = scan_meta_file(r, f);
