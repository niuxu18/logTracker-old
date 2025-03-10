            case APR_SUCCESS:                            /* all fine, just... */
            case APR_EOF:                         /* client closed its end... */
            case APR_TIMEUP:                          /* got bored waiting... */
                rv = 0;                            /* ...gracefully shut down */
                break;
            case APR_EBADF:        /* connection unusable, terminate silently */
            case APR_ECONNABORTED:
                rv = NGHTTP2_ERR_EOF;
                break;
            default:
                break;
        }
    }
    return h2_session_abort_int(session, rv);
}

apr_status_t h2_session_start(h2_session *session, int *rv)
{
    apr_status_t status = APR_SUCCESS;
    h2_config *config;
    nghttp2_settings_entry settings[3];
    
    AP_DEBUG_ASSERT(session);
    /* Start the conversation by submitting our SETTINGS frame */
    *rv = 0;
    config = h2_config_get(session->c);
    if (session->r) {
        const char *s, *cs;
        apr_size_t dlen; 
        h2_stream * stream;

        /* better for vhost matching */
        config = h2_config_rget(session->r);
        
        /* 'h2c' mode: we should have a 'HTTP2-Settings' header with
         * base64 encoded client settings. */
        s = apr_table_get(session->r->headers_in, "HTTP2-Settings");
        if (!s) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, APR_EINVAL, session->r,
                          APLOGNO(02931) 
