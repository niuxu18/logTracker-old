static apr_status_t out_open(h2_mplx *m, int stream_id, h2_response *response,
                             ap_filter_t* f, apr_bucket_brigade *bb,
                             struct apr_thread_cond_t *iowait)
{
    apr_status_t status = APR_SUCCESS;
    
    h2_io *io = h2_io_set_get(m->stream_ios, stream_id);
    if (io && !io->orphaned) {
        if (f) {
            ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                          "h2_mplx(%ld-%d): open response: %d, rst=%d",
                          m->id, stream_id, response->http_status, 
                          response->rst_error);
        }
        
        h2_io_set_response(io, response);
        h2_io_set_add(m->ready_ios, io);
        if (bb) {
            status = out_write(m, io, f, bb, response->trailers, iowait);
        }
        have_out_data_for(m, stream_id);
    }
    else {
        status = APR_ECONNABORTED;
    }
    return status;
}