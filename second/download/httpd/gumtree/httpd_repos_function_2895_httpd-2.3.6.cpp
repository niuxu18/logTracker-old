apr_status_t ap_core_output_filter(ap_filter_t *f, apr_bucket_brigade *new_bb)
{
    conn_rec *c = f->c;
    core_net_rec *net = f->ctx;
    core_output_filter_ctx_t *ctx = net->out_ctx;
    apr_bucket_brigade *bb = NULL;
    apr_bucket *bucket, *next;
    apr_size_t bytes_in_brigade, non_file_bytes_in_brigade;
    apr_status_t rv;

    /* Fail quickly if the connection has already been aborted. */
    if (c->aborted) {
        if (new_bb != NULL) {
            apr_brigade_cleanup(new_bb);
        }
        return APR_ECONNABORTED;
    }

    if (ctx == NULL) {
        ctx = apr_pcalloc(c->pool, sizeof(*ctx));
        net->out_ctx = (core_output_filter_ctx_t *)ctx;
        rv = apr_socket_opt_set(net->client_socket, APR_SO_NONBLOCK, 1);
        if (rv != APR_SUCCESS) {
            return rv;
        }
        /*
         * Need to create tmp brigade with correct lifetime. Passing
         * NULL to apr_brigade_split_ex would result in a brigade
         * allocated from bb->pool which might be wrong.
         */
        ctx->tmp_flush_bb = apr_brigade_create(c->pool, c->bucket_alloc);
    }

    if (new_bb != NULL) {
        for (bucket = APR_BRIGADE_FIRST(new_bb); bucket != APR_BRIGADE_SENTINEL(new_bb); bucket = APR_BUCKET_NEXT(bucket)) {
            if (bucket->length > 0) {
                ctx->bytes_in += bucket->length;
            }
        }
        bb = new_bb;
    }

    if ((ctx->buffered_bb != NULL) &&
        !APR_BRIGADE_EMPTY(ctx->buffered_bb)) {
        if (new_bb != NULL) {
            APR_BRIGADE_PREPEND(bb, ctx->buffered_bb);
        }
        else {
            bb = ctx->buffered_bb;
        }
        c->data_in_output_filters = 0;
    }
    else if (new_bb == NULL) {
        return APR_SUCCESS;
    }

    /* Scan through the brigade and decide whether to attempt a write,
     * based on the following rules:
     *
     *  1) The new_bb is null: Do a nonblocking write of as much as
     *     possible: do a nonblocking write of as much data as possible,
     *     then save the rest in ctx->buffered_bb.  (If new_bb == NULL,
     *     it probably means that the MPM is doing asynchronous write
     *     completion and has just determined that this connection
     *     is writable.)
     *
     *  2) The brigade contains a flush bucket: Do a blocking write
     *     of everything up that point.
     *
     *  3) The request is in CONN_STATE_HANDLER state, and the brigade
     *     contains at least THRESHOLD_MAX_BUFFER bytes in non-file
     *     buckets: Do blocking writes until the amount of data in the
     *     buffer is less than THRESHOLD_MAX_BUFFER.  (The point of this
     *     rule is to provide flow control, in case a handler is
     *     streaming out lots of data faster than the data can be
     *     sent to the client.)
     *
     *  4) The brigade contains at least THRESHOLD_MIN_WRITE
     *     bytes: Do a nonblocking write of as much data as possible,
     *     then save the rest in ctx->buffered_bb.
     */

    if (new_bb == NULL) {
        rv = send_brigade_nonblocking(net->client_socket, bb,
                                      &(ctx->bytes_written), c);
        if (APR_STATUS_IS_EAGAIN(rv)) {
            rv = APR_SUCCESS;
        }
        else if (rv != APR_SUCCESS) {
            /* The client has aborted the connection */
            c->aborted = 1;
        }
        setaside_remaining_output(f, ctx, bb, c);
        return rv;
    }

    bytes_in_brigade = 0;
    non_file_bytes_in_brigade = 0;
    for (bucket = APR_BRIGADE_FIRST(bb); bucket != APR_BRIGADE_SENTINEL(bb);
         bucket = next) {
        next = APR_BUCKET_NEXT(bucket);
        if (APR_BUCKET_IS_FLUSH(bucket)) {
            ctx->tmp_flush_bb = apr_brigade_split_ex(bb, next, ctx->tmp_flush_bb);
            rv = send_brigade_blocking(net->client_socket, bb,
                                       &(ctx->bytes_written), c);
            if (rv != APR_SUCCESS) {
                /* The client has aborted the connection */
                c->aborted = 1;
                return rv;
            }
            APR_BRIGADE_CONCAT(bb, ctx->tmp_flush_bb);
            next = APR_BRIGADE_FIRST(bb);
            bytes_in_brigade = 0;
            non_file_bytes_in_brigade = 0;
        }
        else if (!APR_BUCKET_IS_METADATA(bucket)) {
            if (bucket->length < 0) {
                const char *data;
                apr_size_t length;
                /* XXX support nonblocking read here? */
                rv = apr_bucket_read(bucket, &data, &length, APR_BLOCK_READ);
                if (rv != APR_SUCCESS) {
                    return rv;
                }
                /* reading may have split the bucket, so recompute next: */
                next = APR_BUCKET_NEXT(bucket);
            }
            bytes_in_brigade += bucket->length;
            if (!APR_BUCKET_IS_FILE(bucket)) {
                non_file_bytes_in_brigade += bucket->length;
            }
        }
    }

    if (non_file_bytes_in_brigade >= THRESHOLD_MAX_BUFFER) {
        /* ### Writing the entire brigade may be excessive; we really just
         * ### need to send enough data to be under THRESHOLD_MAX_BUFFER.
         */
        rv = send_brigade_blocking(net->client_socket, bb,
                                   &(ctx->bytes_written), c);
        if (rv != APR_SUCCESS) {
            /* The client has aborted the connection */
            c->aborted = 1;
            return rv;
        }
    }
    else if (bytes_in_brigade >= THRESHOLD_MIN_WRITE) {
        rv = send_brigade_nonblocking(net->client_socket, bb,
                                      &(ctx->bytes_written), c);
        if ((rv != APR_SUCCESS) && (!APR_STATUS_IS_EAGAIN(rv))) {
            /* The client has aborted the connection */
            c->aborted = 1;
            return rv;
        }
    }

    setaside_remaining_output(f, ctx, bb, c);
    return APR_SUCCESS;
}