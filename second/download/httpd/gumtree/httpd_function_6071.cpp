static apr_status_t submit_response(h2_session *session, h2_stream *stream)
{
    apr_status_t status = APR_SUCCESS;
    int rv = 0;
    AP_DEBUG_ASSERT(session);
    AP_DEBUG_ASSERT(stream);
    AP_DEBUG_ASSERT(stream->response || stream->rst_error);
    
    if (stream->submitted) {
        rv = NGHTTP2_PROTOCOL_ERROR;
    }
    else if (stream->response && stream->response->headers) {
        nghttp2_data_provider provider;
        h2_response *response = stream->response;
        h2_ngheader *ngh;
        const h2_priority *prio;
        
        memset(&provider, 0, sizeof(provider));
        provider.source.fd = stream->id;
        provider.read_callback = stream_data_cb;
        
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c,
                      "h2_stream(%ld-%d): submit response %d",
                      session->id, stream->id, response->http_status);
        
        /* If this stream is not a pushed one itself,
         * and HTTP/2 server push is enabled here,
         * and the response is in the range 200-299 *),
         * and the remote side has pushing enabled,
         * -> find and perform any pushes on this stream
         *    *before* we submit the stream response itself.
         *    This helps clients avoid opening new streams on Link
         *    headers that get pushed right afterwards.
         * 
         * *) the response code is relevant, as we do not want to 
         *    make pushes on 401 or 403 codes, neiterh on 301/302
         *    and friends. And if we see a 304, we do not push either
         *    as the client, having this resource in its cache, might
         *    also have the pushed ones as well.
         */
        if (!stream->initiated_on
            && h2_config_geti(session->config, H2_CONF_PUSH)
            && H2_HTTP_2XX(response->http_status)
            && h2_session_push_enabled(session)) {
            
            h2_stream_submit_pushes(stream);
        }
        
        prio = h2_stream_get_priority(stream);
        if (prio) {
            h2_session_set_prio(session, stream, prio);
            /* no showstopper if that fails for some reason */
        }
        
        ngh = h2_util_ngheader_make_res(stream->pool, response->http_status, 
                                        response->headers);
        rv = nghttp2_submit_response(session->ngh2, response->stream_id,
                                     ngh->nv, ngh->nvlen, &provider);
        
    }
    else {
        int err = H2_STREAM_RST(stream, H2_ERR_PROTOCOL_ERROR);
        
        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, session->c,
                      "h2_stream(%ld-%d): RST_STREAM, err=%d",
                      session->id, stream->id, err);

        rv = nghttp2_submit_rst_stream(session->ngh2, NGHTTP2_FLAG_NONE,
                                       stream->id, err);
    }
    
    stream->submitted = 1;

    if (nghttp2_is_fatal(rv)) {
        status = APR_EGENERAL;
        h2_session_abort_int(session, rv);
        ap_log_cerror(APLOG_MARK, APLOG_ERR, status, session->c,
                      APLOGNO(02940) "submit_response: %s", 
                      nghttp2_strerror(rv));
    }
    
    return status;
}