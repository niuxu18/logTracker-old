ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c,
                      "h2_stream(%ld-%d): RST_STREAM, err=%d",
                      session->id, stream->id, err);