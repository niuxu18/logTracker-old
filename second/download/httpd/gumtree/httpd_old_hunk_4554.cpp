     * should be suspended. Beware of trailers.
     */
 
    (void)ng2s;
    (void)buf;
    (void)source;
    stream = get_stream(session, stream_id);
    if (!stream) {
        ap_log_cerror(APLOG_MARK, APLOG_ERR, 0, session->c,
                      APLOGNO(02937) 
                      "h2_stream(%ld-%d): data requested but stream not found",
                      session->id, (int)stream_id);
        return NGHTTP2_ERR_CALLBACK_FAILURE;
    }

    status = h2_stream_out_prepare(stream, &nread, &eos, NULL);
    if (nread) {
        *data_flags |=  NGHTTP2_DATA_FLAG_NO_COPY;
    }
    
    switch (status) {
        case APR_SUCCESS:
            break;
            
        case APR_ECONNRESET:
            return nghttp2_submit_rst_stream(ng2s, NGHTTP2_FLAG_NONE,
                stream->id, stream->rst_error);
            
        case APR_EAGAIN:
            /* If there is no data available, our session will automatically
             * suspend this stream and not ask for more data until we resume
             * it. Remember at our h2_stream that we need to do this.
             */
            nread = 0;
            ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c, APLOGNO(03071)
                          "h2_stream(%ld-%d): suspending",
                          session->id, (int)stream_id);
            return NGHTTP2_ERR_DEFERRED;
            
        default:
            nread = 0;
            ap_log_cerror(APLOG_MARK, APLOG_ERR, status, session->c,
                          APLOGNO(02938) "h2_stream(%ld-%d): reading data",
                          session->id, (int)stream_id);
            return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    
    if (eos) {
        *data_flags |= NGHTTP2_DATA_FLAG_EOF;
    }
    return (ssize_t)nread;
}

struct h2_stream *h2_session_push(h2_session *session, h2_stream *is,
                                  h2_push *push)
{
    apr_status_t status;
    h2_stream *stream;
    h2_ngheader *ngh;
    int nid;
    
    ngh = h2_util_ngheader_make_req(is->pool, push->req);
    nid = nghttp2_submit_push_promise(session->ngh2, 0, is->id, 
                                      ngh->nv, ngh->nvlen, NULL);
    if (nid <= 0) {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c, APLOGNO(03075)
                      "h2_stream(%ld-%d): submitting push promise fail: %s",
                      session->id, is->id, nghttp2_strerror(nid));
        return NULL;
    }
    ++session->pushes_promised;
    
    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c, APLOGNO(03076)
                  "h2_stream(%ld-%d): SERVER_PUSH %d for %s %s on %d",
                  session->id, is->id, nid,
                  push->req->method, push->req->path, is->id);
                  
    stream = h2_session_open_stream(session, nid, is->id, push->req);
    if (stream) {
        h2_session_set_prio(session, stream, push->priority);
        status = stream_schedule(session, stream, 1);
        if (status != APR_SUCCESS) {
            ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, session->c,
                          "h2_stream(%ld-%d): scheduling push stream",
                          session->id, stream->id);
            stream = NULL;
        }
        ++session->unsent_promises;
    }
    else {
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c, APLOGNO(03077)
                      "h2_stream(%ld-%d): failed to create stream obj %d",
                      session->id, is->id, nid);
    }

    if (!stream) {
        /* try to tell the client that it should not wait. */
        nghttp2_submit_rst_stream(session->ngh2, NGHTTP2_FLAG_NONE, nid,
                                  NGHTTP2_INTERNAL_ERROR);
    }
    
    return stream;
}

static int valid_weight(float f) 
{
    int w = (int)f;
