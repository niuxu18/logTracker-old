ap_log_error(APLOG_MARK | APLOG_NOERRNO, APLOG_TRACE7, 0, c->base_server,
                             "mod_dumpio:  %s (%s-%s): %.*s", f->frec->name,
                             (APR_BUCKET_IS_METADATA(b)) ? "metadata" : "data",
                             b->type->name, (int)logbytes, buf);