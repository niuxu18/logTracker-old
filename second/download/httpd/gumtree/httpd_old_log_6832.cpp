ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c, APLOGNO(03456)
                      "h2_session(%ld-%d): denying stream with invalid header "
                      "'%s: %s'", session->id, (int)frame->hd.stream_id,
                      apr_pstrndup(session->pool, (const char *)name, namelen),
                      apr_pstrndup(session->pool, (const char *)value, valuelen));