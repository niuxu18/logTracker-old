    if (errmsg) {
        return errmsg;
    }

    for (i = 0; i < nelts; i++) {
        if (!used->elts[i]) {
            ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_WARNING, 0, NULL,
                         "macro '%s' (%s): argument '%s' (#%d) never used",
                         macro->name, macro->location, names[i], i + 1);
        }
    }

    return NULL;
