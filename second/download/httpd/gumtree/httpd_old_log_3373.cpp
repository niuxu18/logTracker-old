ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r,
                      "ISAPI: ServerSupportFunction HSE_REQ_IO_COMPLETION "
                      "is not supported: %s", r->filename);