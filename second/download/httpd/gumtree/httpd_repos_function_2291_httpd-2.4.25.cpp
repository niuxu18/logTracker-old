static int close_output(h2_stream *stream) 
{
    switch (stream->state) {
        case H2_STREAM_ST_CLOSED_OUTPUT:
        case H2_STREAM_ST_CLOSED:
            return 0; /* ignore, idempotent */
        case H2_STREAM_ST_CLOSED_INPUT:
            /* both closed now */
            set_state(stream, H2_STREAM_ST_CLOSED);
            break;
        default:
            /* everything else we jump to here */
            set_state(stream, H2_STREAM_ST_CLOSED_OUTPUT);
            break;
    }
    return 1;
}