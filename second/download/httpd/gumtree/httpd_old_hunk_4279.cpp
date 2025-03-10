            return h2_config_get_priority(stream->session->config, ctype);
        }
    }
    return NULL;
}

/*******************************************************************************
 * h2_sos_mplx
 ******************************************************************************/

typedef struct h2_sos_mplx {
    h2_mplx *m;
    apr_bucket_brigade *bb;
    apr_bucket_brigade *tmp;
    apr_table_t *trailers;
    apr_off_t  buffer_size;
} h2_sos_mplx;

#define H2_SOS_MPLX_OUT(lvl,msos,msg) \
    do { \
        if (APLOG_C_IS_LEVEL((msos)->m->c,lvl)) \
        h2_util_bb_log((msos)->m->c,(msos)->m->id,lvl,msg,(msos)->bb); \
    } while(0)
    

static apr_status_t mplx_transfer(h2_sos_mplx *msos, int stream_id, 
                                  apr_pool_t *pool)
{
    apr_status_t status;
    apr_table_t *trailers = NULL;
    
    if (!msos->tmp) {
        msos->tmp = apr_brigade_create(msos->bb->p, msos->bb->bucket_alloc);
    }
    status = h2_mplx_out_get_brigade(msos->m, stream_id, msos->tmp, 
                                     msos->buffer_size-1, &trailers);
    if (!APR_BRIGADE_EMPTY(msos->tmp)) {
        h2_transfer_brigade(msos->bb, msos->tmp, pool);
    }
    if (trailers) {
        msos->trailers = trailers;
    }
    return status;
}
 
static apr_status_t h2_sos_mplx_read_to(h2_sos *sos, apr_bucket_brigade *bb, 
                                        apr_off_t *plen, int *peos)
{
    h2_sos_mplx *msos = sos->ctx;
    apr_status_t status;

    status = h2_append_brigade(bb, msos->bb, plen, peos);
    if (status == APR_SUCCESS && !*peos && !*plen) {
        status = APR_EAGAIN;
        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, status, msos->m->c,
                      "h2_stream(%ld-%d): read_to, len=%ld eos=%d",
                      msos->m->id, sos->stream->id, (long)*plen, *peos);
    }
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, msos->m->c,
                  "h2_stream(%ld-%d): read_to, len=%ld eos=%d",
                  msos->m->id, sos->stream->id, (long)*plen, *peos);
    return status;
}

static apr_status_t h2_sos_mplx_readx(h2_sos *sos, h2_io_data_cb *cb, void *ctx,
                                      apr_off_t *plen, int *peos)
{
    h2_sos_mplx *msos = sos->ctx;
    apr_status_t status = APR_SUCCESS;
    
    status = h2_util_bb_readx(msos->bb, cb, ctx, plen, peos);
    if (status == APR_SUCCESS && !*peos && !*plen) {
        status = APR_EAGAIN;
    }
    ap_log_cerror(APLOG_MARK, APLOG_TRACE2, status, msos->m->c,
                  "h2_stream(%ld-%d): readx, len=%ld eos=%d",
                  msos->m->id, sos->stream->id, (long)*plen, *peos);
    return status;
}

static apr_status_t h2_sos_mplx_prepare(h2_sos *sos, apr_off_t *plen, int *peos)
{
    h2_sos_mplx *msos = sos->ctx;
    apr_status_t status = APR_SUCCESS;
    
    H2_SOS_MPLX_OUT(APLOG_TRACE2, msos, "h2_sos_mplx prepare_pre");
    
    if (APR_BRIGADE_EMPTY(msos->bb)) {
        status = mplx_transfer(msos, sos->stream->id, sos->stream->pool);
    }
    h2_util_bb_avail(msos->bb, plen, peos);
    
    H2_SOS_MPLX_OUT(APLOG_TRACE2, msos, "h2_sos_mplx prepare_post");
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, status, msos->m->c,
                  "h2_stream(%ld-%d): prepare, len=%ld eos=%d, trailers=%s",
                  msos->m->id, sos->stream->id, (long)*plen, *peos,
                  msos->trailers? "yes" : "no");
    if (!*peos && !*plen) {
        status = APR_EAGAIN;
    }
    
    return status;
}

static apr_table_t *h2_sos_mplx_get_trailers(h2_sos *sos)
{
    h2_sos_mplx *msos = sos->ctx;

    return msos->trailers;
}

static apr_status_t h2_sos_mplx_buffer(h2_sos *sos, apr_bucket_brigade *bb) 
{
    h2_sos_mplx *msos = sos->ctx;
    apr_status_t status = APR_SUCCESS;

    if (bb && !APR_BRIGADE_EMPTY(bb)) {
        H2_SOS_MPLX_OUT(APLOG_TRACE2, msos, "h2_sos_mplx set_response_pre");
        status = mplx_transfer(msos, sos->stream->id, sos->stream->pool);
        H2_SOS_MPLX_OUT(APLOG_TRACE2, msos, "h2_sos_mplx set_response_post");
    }
    return status;
}

static h2_sos *h2_sos_mplx_create(h2_stream *stream, h2_response *response)
{
    h2_sos *sos;
    h2_sos_mplx *msos;
    
    msos = apr_pcalloc(stream->pool, sizeof(*msos));
    msos->m = stream->session->mplx;
    msos->bb = apr_brigade_create(stream->pool, msos->m->c->bucket_alloc);
    msos->buffer_size = 32 * 1024;
    
    sos = apr_pcalloc(stream->pool, sizeof(*sos));
    sos->stream = stream;
    sos->response = response;
    
    sos->ctx = msos;
    sos->buffer = h2_sos_mplx_buffer;
    sos->prepare = h2_sos_mplx_prepare;
    sos->readx = h2_sos_mplx_readx;
    sos->read_to = h2_sos_mplx_read_to;
    sos->get_trailers = h2_sos_mplx_get_trailers;
    
    sos->response = response;

    return sos;
}

