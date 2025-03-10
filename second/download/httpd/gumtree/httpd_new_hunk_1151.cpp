     */
    if (s->loglevel >= APLOG_DEBUG) {
        X509 *cert  = X509_STORE_CTX_get_current_cert(ctx);
        char *sname = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
        char *iname = X509_NAME_oneline(X509_get_issuer_name(cert),  NULL, 0);

        ap_log_cerror(APLOG_MARK, APLOG_DEBUG, 0, conn,
                      "Certificate Verification: "
                      "depth: %d, subject: %s, issuer: %s",
                      errdepth,
                      sname ? sname : "-unknown-",
                      iname ? iname : "-unknown-");

        if (sname) {
            modssl_free(sname);
        }

        if (iname) {
