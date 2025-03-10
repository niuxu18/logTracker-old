        io->input_consumed += end_len;
        apr_brigade_cleanup(io->bbin);
    }
    return h2_io_in_close(io);
}

apr_status_t h2_io_in_read(h2_io *io, apr_bucket_brigade *bb, 
                           apr_size_t maxlen)
{
    apr_off_t start_len = 0;
    apr_bucket *last;
    apr_status_t status;

    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    
    if (!io->bbin || APR_BRIGADE_EMPTY(io->bbin)) {
        return io->eos_in? APR_EOF : APR_EAGAIN;
    }
    
    apr_brigade_length(bb, 1, &start_len);
    last = APR_BRIGADE_LAST(bb);
    status = h2_util_move(bb, io->bbin, maxlen, NULL, "h2_io_in_read");
    if (status == APR_SUCCESS) {
        apr_bucket *nlast = APR_BRIGADE_LAST(bb);
        apr_off_t end_len = 0;
        apr_brigade_length(bb, 1, &end_len);
        if (last == nlast) {
            return APR_EAGAIN;
        }
        io->input_consumed += (end_len - start_len);
    }
    return status;
}

apr_status_t h2_io_in_write(h2_io *io, apr_bucket_brigade *bb)
{
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    
    if (io->eos_in) {
        return APR_EOF;
    }
    io->eos_in = h2_util_has_eos(bb, -1);
    if (!APR_BRIGADE_EMPTY(bb)) {
        if (!io->bbin) {
            io->bbin = apr_brigade_create(io->pool, io->bucket_alloc);
        }
        return h2_util_move(io->bbin, bb, -1, NULL, "h2_io_in_write");
    }
    return APR_SUCCESS;
}

apr_status_t h2_io_in_close(h2_io *io)
{
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    
    if (io->bbin) {
        APR_BRIGADE_INSERT_TAIL(io->bbin, 
                                apr_bucket_eos_create(io->bbin->bucket_alloc));
    }
    io->eos_in = 1;
    return APR_SUCCESS;
}

apr_status_t h2_io_out_readx(h2_io *io,  
                             h2_io_data_cb *cb, void *ctx, 
                             apr_off_t *plen, int *peos)
{
    apr_status_t status;
    
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    
    if (io->eos_out) {
        *plen = 0;
        *peos = 1;
        return APR_SUCCESS;
    }
    else if (!io->bbout) {
        *plen = 0;
        *peos = 0;
        return APR_EAGAIN;
    }
    
    if (cb == NULL) {
        /* just checking length available */
        status = h2_util_bb_avail(io->bbout, plen, peos);
    }
    else {
        status = h2_util_bb_readx(io->bbout, cb, ctx, plen, peos);
        if (status == APR_SUCCESS) {
            io->eos_out = *peos;
        }
    }
    
    return status;
}

apr_status_t h2_io_out_read_to(h2_io *io, apr_bucket_brigade *bb, 
                               apr_off_t *plen, int *peos)
{
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    
    if (io->eos_out) {
        *plen = 0;
        *peos = 1;
        return APR_SUCCESS;
    }
    else if (!io->bbout) {
        *plen = 0;
        *peos = 0;
        return APR_EAGAIN;
    }

    io->eos_out = *peos = h2_util_has_eos(io->bbout, *plen);
    return h2_util_move(bb, io->bbout, *plen, NULL, "h2_io_read_to");
}

static void process_trailers(h2_io *io, apr_table_t *trailers)
{
    if (trailers && io->response) {
        h2_response_set_trailers(io->response, 
                                 apr_table_clone(io->pool, trailers));
    }
}

apr_status_t h2_io_out_write(h2_io *io, apr_bucket_brigade *bb, 
                             apr_size_t maxlen, apr_table_t *trailers,
                             int *pfile_handles_allowed)
{
    apr_status_t status;
    int start_allowed;
    
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }

    if (io->eos_out) {
        apr_off_t len;
        /* We have already delivered an EOS bucket to a reader, no
         * sense in storing anything more here.
         */
        status = apr_brigade_length(bb, 1, &len);
        if (status == APR_SUCCESS) {
            if (len > 0) {
                /* someone tries to write real data after EOS, that
                 * does not look right. */
                status = APR_EOF;
            }
            /* cleanup, as if we had moved the data */
            apr_brigade_cleanup(bb);
        }
        return status;
    }

    process_trailers(io, trailers);
    if (!io->bbout) {
        io->bbout = apr_brigade_create(io->pool, io->bucket_alloc);
    }
    
    /* Let's move the buckets from the request processing in here, so
     * that the main thread can read them when it has time/capacity.
     *
     * Move at most "maxlen" memory bytes. If buckets remain, it is
     * the caller's responsibility to take care of this.
     *
     * We allow passing of file buckets as long as we do not have too
     * many open files already buffered. Otherwise we will run out of
     * file handles.
     */
    start_allowed = *pfile_handles_allowed;
    status = h2_util_move(io->bbout, bb, maxlen, pfile_handles_allowed, 
                          "h2_io_out_write");
    /* track # file buckets moved into our pool */
    if (start_allowed != *pfile_handles_allowed) {
        io->files_handles_owned += (start_allowed - *pfile_handles_allowed);
    }
    return status;
}


apr_status_t h2_io_out_close(h2_io *io, apr_table_t *trailers)
{
    if (io->rst_error) {
        return APR_ECONNABORTED;
    }
    if (!io->eos_out) { /* EOS has not been read yet */
        process_trailers(io, trailers);
        if (!io->bbout) {
            io->bbout = apr_brigade_create(io->pool, io->bucket_alloc);
        }
        if (!h2_util_has_eos(io->bbout, -1)) {
            APR_BRIGADE_INSERT_TAIL(io->bbout, 
                                    apr_bucket_eos_create(io->bbout->bucket_alloc));
        }
    }
    return APR_SUCCESS;
}
