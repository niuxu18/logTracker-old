apr_status_t h2_mplx_dispatch_master_events(h2_mplx *m, 
                                            stream_ev_callback *on_resume, 
                                            void *on_ctx)
{
    h2_stream *stream;
    int n, id;
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, m->c, 
                  "h2_mplx(%ld): dispatch events", m->id);        
    apr_atomic_set32(&m->event_pending, 0);

    /* update input windows for streams */
    h2_ihash_iter(m->streams, report_consumption_iter, m);    
    purge_streams(m, 1);
    
    n = h2_ififo_count(m->readyq);
    while (n > 0 
           && (h2_ififo_try_pull(m->readyq, &id) == APR_SUCCESS)) {
        --n;
        stream = h2_ihash_get(m->streams, id);
        if (stream) {
            on_resume(on_ctx, stream);
        }
    }
    
    return APR_SUCCESS;
}