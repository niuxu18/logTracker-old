     * should take place. This cannot work.
     */
    if (mctx->auth.verify_mode == SSL_CVERIFY_REQUIRE) {
        ca_list = SSL_CTX_get_client_CA_list(ctx);

        if (sk_X509_NAME_num(ca_list) == 0) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, s, APLOGNO(01897)
                         "Init: Oops, you want to request client "
                         "authentication, but no CAs are known for "
                         "verification!?  [Hint: SSLCACertificate*]");
        }
    }
}
