apr_status_t h2_session_stream_destroy(h2_session *session, h2_stream *stream)
{
    apr_pool_t *pool = h2_stream_detach_pool(stream);

    /* this may be called while the session has already freed
     * some internal structures. */
    if (session->mplx) {
        h2_mplx_stream_done(session->mplx, stream->id, stream->rst_error);
        if (session->last_stream == stream) {
            session->last_stream = NULL;
        }
    }
    
    if (session->streams) {
        h2_stream_set_remove(session->streams, stream->id);
    }
    h2_stream_destroy(stream);
    
    if (pool) {
        apr_pool_clear(pool);
        if (session->spare) {
            apr_pool_destroy(session->spare);
        }
        session->spare = pool;
    }
    return APR_SUCCESS;
}