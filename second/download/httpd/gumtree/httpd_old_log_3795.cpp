ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "execution failure "
                              "for parameter \"%s\" to tag exec in file %s",
                              tag, r->filename);