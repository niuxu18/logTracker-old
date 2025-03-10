 *   Since HTTP/2 connections can be expected to live longer than
 *   their HTTP/1 cousins, the separate allocator seems to work better
 *   than protecting a shared h2_session one with an own lock.
 */
h2_mplx *h2_mplx_create(conn_rec *c, apr_pool_t *parent, 
                        const h2_config *conf, 
                        apr_interval_time_t stream_timeout,
                        h2_workers *workers)
{
    apr_status_t status = APR_SUCCESS;
    apr_allocator_t *allocator = NULL;
    h2_mplx *m;
    ap_assert(conf);
    
    status = apr_allocator_create(&allocator);
    if (status != APR_SUCCESS) {
        return NULL;
    }

    m = apr_pcalloc(parent, sizeof(h2_mplx));
    if (m) {
        m->id = c->id;
        APR_RING_ELEM_INIT(m, link);
        m->c = c;
        apr_pool_create_ex(&m->pool, parent, NULL, allocator);
        if (!m->pool) {
            return NULL;
        }
        apr_pool_tag(m->pool, "h2_mplx");
        apr_allocator_owner_set(allocator, m->pool);
        
        status = apr_thread_mutex_create(&m->lock, APR_THREAD_MUTEX_DEFAULT,
                                         m->pool);
        if (status != APR_SUCCESS) {
            h2_mplx_destroy(m);
            return NULL;
        }
        
        status = apr_thread_cond_create(&m->task_thawed, m->pool);
        if (status != APR_SUCCESS) {
            h2_mplx_destroy(m);
            return NULL;
        }
    
        m->bucket_alloc = apr_bucket_alloc_create(m->pool);
        m->max_streams = h2_config_geti(conf, H2_CONF_MAX_STREAMS);
        m->stream_max_mem = h2_config_geti(conf, H2_CONF_STREAM_MAX_MEM);

        m->streams = h2_ihash_create(m->pool, offsetof(h2_stream,id));
        m->shold = h2_ihash_create(m->pool, offsetof(h2_stream,id));
        m->spurge = h2_ihash_create(m->pool, offsetof(h2_stream,id));
        m->q = h2_iq_create(m->pool, m->max_streams);
        m->readyq = h2_iq_create(m->pool, m->max_streams);
        m->tasks = h2_ihash_create(m->pool, offsetof(h2_task,stream_id));
        m->redo_tasks = h2_ihash_create(m->pool, offsetof(h2_task, stream_id));

        m->stream_timeout = stream_timeout;
        m->workers = workers;
        m->workers_max = workers->max_workers;
        m->workers_limit = 6; /* the original h1 max parallel connections */
        m->last_limit_change = m->last_idle_block = apr_time_now();
        m->limit_change_interval = apr_time_from_msec(200);
        
        m->tx_handles_reserved = 0;
        m->tx_chunk_size = 4;
        
        m->spare_slaves = apr_array_make(m->pool, 10, sizeof(conn_rec*));
        
        m->ngn_shed = h2_ngn_shed_create(m->pool, m->c, m->max_streams, 
                                         m->stream_max_mem);
        h2_ngn_shed_set_ctx(m->ngn_shed , m);
    }
    return m;
}

int h2_mplx_shutdown(h2_mplx *m)
{
    int acquired, max_stream_started = 0;
    
    if (enter_mutex(m, &acquired) == APR_SUCCESS) {
        max_stream_started = m->max_stream_started;
        /* Clear schedule queue, disabling existing streams from starting */ 
        h2_iq_clear(m->q);
        leave_mutex(m, acquired);
    }
    return max_stream_started;
}

static void input_consumed_signal(h2_mplx *m, h2_stream *stream)
{
    if (stream->input && stream->started) {
        h2_beam_send(stream->input, NULL, 0); /* trigger updates */
    }
}

static int output_consumed_signal(h2_mplx *m, h2_task *task)
{
    if (task->output.beam && task->worker_started && task->assigned) {
        /* trigger updates */
        h2_beam_send(task->output.beam, NULL, APR_NONBLOCK_READ);
    }
    return 0;
}


static void task_destroy(h2_mplx *m, h2_task *task, int called_from_master)
{
    conn_rec *slave = NULL;
    int reuse_slave = 0;
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE3, 0, m->c, 
                  "h2_task(%s): destroy", task->id);
    if (called_from_master) {
        /* Process outstanding events before destruction */
        h2_stream *stream = h2_ihash_get(m->streams, task->stream_id);
        if (stream) {
            input_consumed_signal(m, stream);
        }
    }
    
    h2_beam_on_produced(task->output.beam, NULL, NULL);
    ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, m->c, 
                  APLOGNO(03385) "h2_task(%s): destroy "
                  "output beam empty=%d, holds proxies=%d", 
                  task->id,
                  h2_beam_empty(task->output.beam),
                  h2_beam_holds_proxies(task->output.beam));
    
    slave = task->c;
    reuse_slave = ((m->spare_slaves->nelts < m->spare_slaves->nalloc)
                   && !task->rst_error);
    
    h2_ihash_remove(m->tasks, task->stream_id);
    h2_ihash_remove(m->redo_tasks, task->stream_id);
    h2_task_destroy(task);

    if (slave) {
        if (reuse_slave && slave->keepalive == AP_CONN_KEEPALIVE) {
            APR_ARRAY_PUSH(m->spare_slaves, conn_rec*) = slave;
        }
        else {
            slave->sbh = NULL;
            h2_slave_destroy(slave);
        }
    }
    
    check_tx_free(m);
}

static void stream_done(h2_mplx *m, h2_stream *stream, int rst_error) 
{
    h2_task *task;
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE3, 0, m->c, 
                  "h2_stream(%ld-%d): done", m->c->id, stream->id);
    /* Situation: we are, on the master connection, done with processing
     * the stream. Either we have handled it successfully, or the stream
     * was reset by the client or the connection is gone and we are 
     * shutting down the whole session.
     *
     * We possibly have created a task for this stream to be processed
     * on a slave connection. The processing might actually be ongoing
     * right now or has already finished. A finished task waits for its
     * stream to be done. This is the common case.
     * 
     * If the stream had input (e.g. the request had a body), a task
     * may have read, or is still reading buckets from the input beam.
     * This means that the task is referencing memory from the stream's
     * pool (or the master connection bucket alloc). Before we can free
     * the stream pool, we need to make sure that those references are
     * gone. This is what h2_beam_shutdown() on the input waits for.
     *
     * With the input handled, we can tear down that beam and care
     * about the output beam. The stream might still have buffered some
     * buckets read from the output, so we need to get rid of those. That
     * is done by h2_stream_cleanup().
     *
     * Now it is save to destroy the task (if it exists and is finished).
     * 
     * FIXME: we currently destroy the stream, even if the task is still
     * ongoing. This is not ok, since task->request is coming from stream
     * memory. We should either copy it on task creation or wait with the
     * stream destruction until the task is done. 
     */
    h2_iq_remove(m->q, stream->id);
    h2_ihash_remove(m->streams, stream->id);
    
    h2_stream_cleanup(stream);
    m->tx_handles_reserved += h2_beam_get_files_beamed(stream->input);
    h2_beam_on_consumed(stream->input, NULL, NULL);
    /* Let anyone blocked reading know that there is no more to come */
    h2_beam_abort(stream->input);
    /* Remove mutex after, so that abort still finds cond to signal */
    h2_beam_mutex_set(stream->input, NULL, NULL, NULL);
    m->tx_handles_reserved += h2_beam_get_files_beamed(stream->output);

    task = h2_ihash_get(m->tasks, stream->id);
    if (task) {
        if (!task->worker_done) {
            /* task still running, cleanup once it is done */
            if (rst_error) {
                h2_task_rst(task, rst_error);
            }
            h2_ihash_add(m->shold, stream);
            return;
        }
        else {
            /* already finished */
            task_destroy(m, task, 1);
        }
    }
    h2_stream_destroy(stream);
}

static int stream_done_iter(void *ctx, void *val)
{
    stream_done((h2_mplx*)ctx, val, 0);
    return 0;
}

typedef struct {
    h2_mplx_stream_cb *cb;
    void *ctx;
} stream_iter_ctx_t;
