ap_log_cerror(APLOG_MARK, nghttp2_is_fatal(rv)?
                          APLOG_ERR : APLOG_DEBUG, 0, session->c,
                          APLOGNO(02936) 
                          "h2_stream(%ld-%d): resuming %s, len=%ld, eos=%d",
                          session->id, stream->id, 
                          rv? nghttp2_strerror(rv) : "", (long)len, eos);