ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, stream->m->c,
                  "h2_stream(%ld-%d): closing input",
                  stream->m->id, stream->id);