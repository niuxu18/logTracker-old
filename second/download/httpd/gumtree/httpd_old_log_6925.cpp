ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, c,
                  "h2_stream(%ld-%d): prepare, len=%ld eos=%d",
                  c->id, stream->id, (long)*plen, *peos);