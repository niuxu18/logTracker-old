ap_log_error(APLOG_MARK, APLOG_ERR, rv, s,
                     "Cannot open socache DBM file `%s' for writing "
                     "(store)",
                     ctx->data_file);