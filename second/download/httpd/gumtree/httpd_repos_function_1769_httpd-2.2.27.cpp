static int dumpio_output_filter (ap_filter_t *f, apr_bucket_brigade *bb)
{
    apr_bucket *b;
    conn_rec *c = f->c;
    dumpio_conf_t *ptr =
    (dumpio_conf_t *) ap_get_module_config(c->base_server->module_config,
                                           &dumpio_module);

    ap_log_error(APLOG_MARK, ptr->loglevel, 0, c->base_server, "mod_dumpio: %s", f->frec->name) ;

    for (b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b)) {
        /*
         * If we ever see an EOS, make sure to FLUSH.
         */
        if (APR_BUCKET_IS_EOS(b)) {
            apr_bucket *flush = apr_bucket_flush_create(f->c->bucket_alloc);
            APR_BUCKET_INSERT_BEFORE(b, flush);
        }
        dumpit(f, b);
    }

    return ap_pass_brigade(f->next, bb) ;
}