apr_table_t *h2_stream_get_trailers(h2_stream *stream)
{
    return stream->response? stream->response->trailers : NULL;
}