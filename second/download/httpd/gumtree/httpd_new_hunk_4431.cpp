        next = APR_BUCKET_NEXT(b);
        if (b->length == 0 && !APR_BUCKET_IS_METADATA(b)) {
            apr_bucket_delete(b);
        } 
    }
    
    while (APR_BRIGADE_EMPTY(task->input.bb)) {
        /* Get more input data for our request. */
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, f->c,
                      "h2_slave_in(%s): get more data from mplx, block=%d, "
                      "readbytes=%ld", task->id, block, (long)readbytes);
        
        /* Override the block mode we get called with depending on the input's
         * setting. */
        if (task->input.beam) {
            status = h2_beam_receive(task->input.beam, task->input.bb, block, 
                                     H2MIN(readbytes, 32*1024));
        }
        else {
            status = APR_EOF;
        }
        
        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, status, f->c,
                      "h2_slave_in(%s): read returned", task->id);
        if (APR_STATUS_IS_EAGAIN(status) 
            && (mode == AP_MODE_GETLINE || block == APR_BLOCK_READ)) {
            /* chunked input handling does not seem to like it if we
             * return with APR_EAGAIN from a GETLINE read... 
             * upload 100k test on test-ser.example.org hangs */
            status = APR_SUCCESS;
        }
        else if (APR_STATUS_IS_EOF(status)) {
            break;
        }
        else if (status != APR_SUCCESS) {
            return status;
        }
        
        h2_util_bb_log(f->c, task->stream_id, APLOG_TRACE2, 
                       "input.beam recv raw", task->input.bb);
        if (h2_task_logio_add_bytes_in) {
            apr_brigade_length(bb, 0, &bblen);
            h2_task_logio_add_bytes_in(f->c, bblen);
        }
    }
    
    /* Inspect the buckets received, detect EOS and apply
     * chunked encoding if necessary */
    if (status == APR_EOF && APR_BRIGADE_EMPTY(task->input.bb)) {
        return APR_EOF;
    }

    h2_util_bb_log(f->c, task->stream_id, APLOG_TRACE2, 
                   "task_input.bb", task->input.bb);
           
    if (APR_BRIGADE_EMPTY(task->input.bb)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, f->c,
                      "h2_slave_in(%s): no data", task->id);
        return (block == APR_NONBLOCK_READ)? APR_EAGAIN : APR_EOF;
    }
    
    if (mode == AP_MODE_EXHAUSTIVE) {
        /* return all we have */
        APR_BRIGADE_CONCAT(bb, task->input.bb);
    }
    else if (mode == AP_MODE_READBYTES) {
        status = h2_brigade_concat_length(bb, task->input.bb, rmax);
    }
    else if (mode == AP_MODE_SPECULATIVE) {
        status = h2_brigade_copy_length(bb, task->input.bb, rmax);
    }
    else if (mode == AP_MODE_GETLINE) {
        /* we are reading a single LF line, e.g. the HTTP headers. 
         * this has the nasty side effect to split the bucket, even
         * though it ends with CRLF and creates a 0 length bucket */
        status = apr_brigade_split_line(bb, task->input.bb, block, 
