int h2_h2_process_conn(conn_rec* c)
{
    h2_ctx *ctx = h2_ctx_get(c);
    h2_config *cfg = h2_config_get(c);
    apr_bucket_brigade* temp;
    int is_tls = h2_h2_is_tls(c);
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, c, "h2_h2, process_conn");
    if (h2_ctx_is_task(ctx)) {
        /* our stream pseudo connection */
        return DECLINED;
    }

    /* If we have not already switched to a h2* protocol and the connection 
     * is on "http/1.1"
     * -> sniff for the magic PRIamble. On TLS, this might trigger the ALPN.
     */
    if (!h2_ctx_protocol_get(c) 
        && !strcmp(AP_PROTOCOL_HTTP1, ap_get_protocol(c))) {
        apr_status_t status;
        
        temp = apr_brigade_create(c->pool, c->bucket_alloc);
        status = ap_get_brigade(c->input_filters, temp,
                                AP_MODE_SPECULATIVE, APR_BLOCK_READ, 24);

        if (status == APR_SUCCESS) {
            if (h2_ctx_protocol_get(c) 
                || strcmp(AP_PROTOCOL_HTTP1, ap_get_protocol(c))) {
                /* h2 or another protocol has been selected. */
            }
            else {
                /* ALPN might have been triggered, but we're still on
                 * http/1.1. Check the actual bytes read for the H2 Magic
                 * Token, *if* H2Direct mode is enabled here. 
                 */
                int direct_mode = h2_config_geti(cfg, H2_CONF_DIRECT);
                if (direct_mode > 0 || (direct_mode < 0 && !is_tls)) {
                    char *s = NULL;
                    apr_size_t slen;
                    
                    apr_brigade_pflatten(temp, &s, &slen, c->pool);
                    if ((slen >= 24) && !memcmp(H2_MAGIC_TOKEN, s, 24)) {
                        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, c,
                                      "h2_h2, direct mode detected");
                        h2_ctx_protocol_set(ctx, is_tls? "h2" : "h2c");
                    }
                    else {
                        ap_log_cerror(APLOG_MARK, APLOG_TRACE2, 0, c,
                                      "h2_h2, not detected in %d bytes: %s", 
                                      (int)slen, s);
                    }
                }
            }
        }
        else {
            ap_log_cerror(APLOG_MARK, APLOG_DEBUG, status, c,
                          "h2_h2, error reading 24 bytes speculative");
        }
        apr_brigade_destroy(temp);
    }

    /* If "h2" was selected as protocol (by whatever mechanism), take over
     * the connection.
     */
    if (h2_ctx_is_active(ctx)) {
        ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, c,
                      "h2_h2, connection, h2 active");
        
        return h2_conn_main(c);
    }
    
    ap_log_cerror(APLOG_MARK, APLOG_TRACE1, 0, c, "h2_h2, declined");
    return DECLINED;
}