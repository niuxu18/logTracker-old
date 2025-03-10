apr_status_t h2_stream_set_request(h2_stream *stream, request_rec *r)
{
    apr_status_t status;
    AP_DEBUG_ASSERT(stream);
    if (stream->rst_error) {
        return APR_ECONNRESET;
    }
    set_state(stream, H2_STREAM_ST_OPEN);
    status = h2_request_rwrite(stream->request, r);
    stream->request->serialize = h2_config_geti(h2_config_rget(r), 
                                                H2_CONF_SER_HEADERS);

    return status;
}