    
    ctx = h2_ctx_get(c, 0);
    (void)arg;
    if (h2_ctx_is_task(ctx)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, c,
                      "h2_h2, pre_connection, found stream task");
        ap_add_input_filter("H2_SLAVE_IN", NULL, NULL, c);
        ap_add_output_filter("H2_PARSE_H1", NULL, NULL, c);
        ap_add_output_filter("H2_SLAVE_OUT", NULL, NULL, c);
    }
    return OK;
}

h2_task *h2_task_create(conn_rec *c, int stream_id, const h2_request *req, 
                        h2_bucket_beam *input, h2_bucket_beam *output,  
                        h2_mplx *mplx)
{
    apr_pool_t *pool;
    h2_task *task;
    
    ap_assert(mplx);
    ap_assert(c);
    ap_assert(req);

    apr_pool_create(&pool, c->pool);
    task = apr_pcalloc(pool, sizeof(h2_task));
    if (task == NULL) {
        ap_log_cerror(APLOG_MARK, APLOG_ERR, APR_ENOMEM, c,
                      APLOGNO(02941) "h2_task(%ld-%d): create stream task", 
                      c->id, stream_id);
        return NULL;
    }
    task->id          = apr_psprintf(pool, "%ld-%d", c->master->id, stream_id);
    task->stream_id   = stream_id;
    task->c           = c;
    task->mplx        = mplx;
    task->c->keepalives = mplx->c->keepalives;
    task->pool        = pool;
    task->request     = req;
    task->input.beam  = input;
    task->output.beam = output;
    
    apr_thread_cond_create(&task->cond, pool);

    h2_ctx_create_for(c, task);
    return task;
}

void h2_task_destroy(h2_task *task)
{
    if (task->eor) {
        apr_bucket_destroy(task->eor);
    }
    if (task->pool) {
        apr_pool_destroy(task->pool);
    }
}

apr_status_t h2_task_do(h2_task *task, apr_thread_t *thread, int worker_id)
{
    ap_assert(task);

    if (task->c->master) {
        /* Each conn_rec->id is supposed to be unique at a point in time. Since
         * some modules (and maybe external code) uses this id as an identifier
         * for the request_rec they handle, it needs to be unique for slave 
         * connections also.
         * The connection id is generated by the MPM and most MPMs use the formula
         *    id := (child_num * max_threads) + thread_num
         * which means that there is a maximum id of about
         *    idmax := max_child_count * max_threads
         * If we assume 2024 child processes with 2048 threads max, we get
         *    idmax ~= 2024 * 2048 = 2 ** 22
         * On 32 bit systems, we have not much space left, but on 64 bit systems
         * (and higher?) we can use the upper 32 bits without fear of collision.
         * 32 bits is just what we need, since a connection can only handle so
         * many streams. 
         */
        int slave_id, free_bits;
        
        if (sizeof(unsigned long) >= 8) {
            free_bits = 32;
            slave_id = task->stream_id;
        }
        else {
            /* Assume we have a more limited number of threads/processes
             * and h2 workers on a 32-bit system. Use the worker instead
             * of the stream id. */
            free_bits = 8;
            slave_id = worker_id; 
        }
        task->c->id = (task->c->master->id << free_bits)^slave_id;
    }
    
    task->input.bb = apr_brigade_create(task->pool, task->c->bucket_alloc);
    if (task->request->serialize) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, task->c,
                      "h2_task(%s): serialize request %s %s", 
                      task->id, task->request->method, task->request->path);
        apr_brigade_printf(task->input.bb, NULL, 
                           NULL, "%s %s HTTP/1.1\r\n", 
                           task->request->method, task->request->path);
        apr_table_do(input_ser_header, task, task->request->headers, NULL);
        apr_brigade_puts(task->input.bb, NULL, NULL, "\r\n");
    }
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, task->c,
                  "h2_task(%s): process connection", task->id);
    task->c->current_thread = thread; 
    ap_run_process_connection(task->c);
    
