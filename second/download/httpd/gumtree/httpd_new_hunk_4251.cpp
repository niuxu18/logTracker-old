    NGH2_SET_CALLBACK(*pcb, send_data, on_send_data_cb);
    NGH2_SET_CALLBACK(*pcb, on_frame_send, on_frame_send_cb);

    return APR_SUCCESS;
}

static void h2_session_destroy(h2_session *session)
{
    AP_DEBUG_ASSERT(session);    

    h2_ihash_clear(session->streams);
    if (session->mplx) {
        h2_mplx_set_consumed_cb(session->mplx, NULL, NULL);
        h2_mplx_release_and_join(session->mplx, session->iowait);
        session->mplx = NULL;
    }

    ap_remove_input_filter_byhandle((session->r? session->r->input_filters :
                                     session->c->input_filters), "H2_IN");
    if (session->ngh2) {
        nghttp2_session_del(session->ngh2);
        session->ngh2 = NULL;
    }
    if (session->c) {
        h2_ctx_clear(session->c);
    }

    if (APLOGctrace1(session->c)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, session->c,
                      "h2_session(%ld): destroy", session->id);
    }
    if (session->pool) {
        apr_pool_destroy(session->pool);
    }
}

