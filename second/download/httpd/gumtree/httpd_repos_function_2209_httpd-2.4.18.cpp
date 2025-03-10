apr_status_t h2_task_input_read(h2_task_input *input,
                                ap_filter_t* f,
                                apr_bucket_brigade* bb,
                                ap_input_mode_t mode,
                                apr_read_type_e block,
                                apr_off_t readbytes)
{
    apr_status_t status = APR_SUCCESS;
    apr_off_t bblen = 0;
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                  "h2_task_input(%s): read, block=%d, mode=%d, readbytes=%ld", 
                  input->task->id, block, mode, (long)readbytes);
    
    if (mode == AP_MODE_INIT) {
        return ap_get_brigade(f->c->input_filters, bb, mode, block, readbytes);
    }
    
    if (is_aborted(f)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                      "h2_task_input(%s): is aborted", input->task->id);
        return APR_ECONNABORTED;
    }
    
    if (input->bb) {
        status = apr_brigade_length(input->bb, 1, &bblen);
        if (status != APR_SUCCESS) {
            ap_log_cerror(APLOG_MARK, APLOG_WARNING, status, f->c,
                          APLOGNO(02958) "h2_task_input(%s): brigade length fail", 
                          input->task->id);
            return status;
        }
    }
    
    if ((bblen == 0) && input->task->input_eos) {
        return APR_EOF;
    }
    
    while ((bblen == 0) || (mode == AP_MODE_READBYTES && bblen < readbytes)) {
        /* Get more data for our stream from mplx.
         */
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                      "h2_task_input(%s): get more data from mplx, block=%d, "
                      "readbytes=%ld, queued=%ld",
                      input->task->id, block, 
                      (long)readbytes, (long)bblen);
        
        /* Although we sometimes get called with APR_NONBLOCK_READs, 
         we seem to  fill our buffer blocking. Otherwise we get EAGAIN,
         return that to our caller and everyone throws up their hands,
         never calling us again. */
        status = h2_mplx_in_read(input->task->mplx, APR_BLOCK_READ,
                                 input->task->stream_id, input->bb, 
                                 input->task->io);
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                      "h2_task_input(%s): mplx in read returned",
                      input->task->id);
        if (status != APR_SUCCESS) {
            return status;
        }
        status = apr_brigade_length(input->bb, 1, &bblen);
        if (status != APR_SUCCESS) {
            return status;
        }
        if ((bblen == 0) && (block == APR_NONBLOCK_READ)) {
            return h2_util_has_eos(input->bb, -1)? APR_EOF : APR_EAGAIN;
        }
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                      "h2_task_input(%s): mplx in read, %ld bytes in brigade",
                      input->task->id, (long)bblen);
    }
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                  "h2_task_input(%s): read, mode=%d, block=%d, "
                  "readbytes=%ld, queued=%ld",
                  input->task->id, mode, block, 
                  (long)readbytes, (long)bblen);
           
    if (!APR_BRIGADE_EMPTY(input->bb)) {
        if (mode == AP_MODE_EXHAUSTIVE) {
            /* return all we have */
            return h2_util_move(bb, input->bb, readbytes, NULL, 
                                "task_input_read(exhaustive)");
        }
        else if (mode == AP_MODE_READBYTES) {
            return h2_util_move(bb, input->bb, readbytes, NULL, 
                                "task_input_read(readbytes)");
        }
        else if (mode == AP_MODE_SPECULATIVE) {
            /* return not more than was asked for */
            return h2_util_copy(bb, input->bb, readbytes,  
                                "task_input_read(speculative)");
        }
        else if (mode == AP_MODE_GETLINE) {
            /* we are reading a single LF line, e.g. the HTTP headers */
            status = apr_brigade_split_line(bb, input->bb, block, 
                                            HUGE_STRING_LEN);
            if (APLOGctrace1(f->c)) {
                char buffer[1024];
                apr_size_t len = sizeof(buffer)-1;
                apr_brigade_flatten(bb, buffer, &len);
                buffer[len] = 0;
                ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                              "h2_task_input(%s): getline: %s",
                              input->task->id, buffer);
            }
            return status;
        }
        else {
            /* Hmm, well. There is mode AP_MODE_EATCRLF, but we chose not
             * to support it. Seems to work. */
            ap_log_cerror(APLOG_MARK, APLOG_ERR, APR_ENOTIMPL, f->c,
                          APLOGNO(02942) 
                          "h2_task_input, unsupported READ mode %d", mode);
            return APR_ENOTIMPL;
        }
    }
    
    if (is_aborted(f)) {
        return APR_ECONNABORTED;
    }
    
    return (block == APR_NONBLOCK_READ)? APR_EAGAIN : APR_EOF;
}