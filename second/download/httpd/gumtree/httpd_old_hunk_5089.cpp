static void ssl_init_server_certs(server_rec *s,
                                  apr_pool_t *p,
                                  apr_pool_t *ptemp,
                                  modssl_ctx_t *mctx)
{
    const char *rsa_id, *dsa_id;
#ifndef OPENSSL_NO_EC
    const char *ecc_id;
#endif
    const char *vhost_id = mctx->sc->vhost_id;
    int i;
    int have_rsa, have_dsa;
#ifndef OPENSSL_NO_EC
    int have_ecc;
#endif

    rsa_id = ssl_asn1_table_keyfmt(ptemp, vhost_id, SSL_AIDX_RSA);
    dsa_id = ssl_asn1_table_keyfmt(ptemp, vhost_id, SSL_AIDX_DSA);
#ifndef OPENSSL_NO_EC
    ecc_id = ssl_asn1_table_keyfmt(ptemp, vhost_id, SSL_AIDX_ECC);
#endif

    have_rsa = ssl_server_import_cert(s, mctx, rsa_id, SSL_AIDX_RSA);
    have_dsa = ssl_server_import_cert(s, mctx, dsa_id, SSL_AIDX_DSA);
#ifndef OPENSSL_NO_EC
    have_ecc = ssl_server_import_cert(s, mctx, ecc_id, SSL_AIDX_ECC);
#endif

    if (!(have_rsa || have_dsa
#ifndef OPENSSL_NO_EC
        || have_ecc
#endif
)) {
        ap_log_error(APLOG_MARK, APLOG_EMERG, 0, s, APLOGNO(01910)
                "Oops, no " KEYTYPES " server certificate found "
                "for '%s:%d'?!", s->server_hostname, s->port);
