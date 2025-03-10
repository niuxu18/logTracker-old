                          "h2_conn_io: error reading");
            break;
    }
    return status;
}

static apr_status_t pass_out(apr_bucket_brigade *bb, void *ctx) 
{
    h2_conn_io *io = (h2_conn_io*)ctx;
    apr_status_t status;
    apr_off_t bblen;
    
    if (APR_BRIGADE_EMPTY(bb)) {
        return APR_SUCCESS;
    }
    
    ap_update_child_status(io->connection->sbh, SERVER_BUSY_WRITE, NULL);
    status = apr_brigade_length(bb, 0, &bblen);
    if (status == APR_SUCCESS) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, io->connection,
                      "h2_conn_io(%ld): pass_out brigade %ld bytes",
                      io->connection->id, (long)bblen);
        status = ap_pass_brigade(io->connection->output_filters, bb);
        if (status == APR_SUCCESS) {
            io->bytes_written += (apr_size_t)bblen;
            io->last_write = apr_time_now();
        }
        apr_brigade_cleanup(bb);
    }
    return status;
}

/* Bring the current buffer content into the output brigade, appropriately
 * chunked.
 */
static apr_status_t bucketeer_buffer(h2_conn_io *io) {
    const char *data = io->buffer;
    apr_size_t remaining = io->buflen;
    apr_bucket *b;
    int bcount, i;

    if (io->write_size > WRITE_SIZE_INITIAL 
        && (io->cooldown_usecs > 0)
        && (apr_time_now() - io->last_write) >= io->cooldown_usecs) {
        /* long time not written, reset write size */
        io->write_size = WRITE_SIZE_INITIAL;
        io->bytes_written = 0;
        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, io->connection,
                      "h2_conn_io(%ld): timeout write size reset to %ld", 
                      (long)io->connection->id, (long)io->write_size);
    }
    else if (io->write_size < WRITE_SIZE_MAX 
             && io->bytes_written >= io->warmup_size) {
        /* connection is hot, use max size */
        io->write_size = WRITE_SIZE_MAX;
        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, io->connection,
                      "h2_conn_io(%ld): threshold reached, write size now %ld", 
                      (long)io->connection->id, (long)io->write_size);
    }
