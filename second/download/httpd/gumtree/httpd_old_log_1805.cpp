ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                                  "inflate: bad flags %02x",
                                  ctx->zlib_flags);