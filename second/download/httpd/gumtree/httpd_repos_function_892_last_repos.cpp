static int ssl_auth_compatible(modssl_auth_ctx_t *a1, 
                               modssl_auth_ctx_t *a2) 
{
    if (!a1 || !a2) {
        return 0;
    }
    /* both have the same verification */
    if ((a1->verify_depth != a2->verify_depth)
        || (a1->verify_mode != a2->verify_mode)) {
        return 0;
    }
    /* both have the same ca path/file */
    if ((a1->ca_cert_path != a2->ca_cert_path)
        && (!a1->ca_cert_path || !a2->ca_cert_path 
            || strcmp(a1->ca_cert_path, a2->ca_cert_path))) {
        return 0;
    }
    if ((a1->ca_cert_file != a2->ca_cert_file)
        && (!a1->ca_cert_file || !a2->ca_cert_file 
            || strcmp(a1->ca_cert_file, a2->ca_cert_file))) {
        return 0;
    }
    /* both have the same ca cipher suite string */
    if ((a1->cipher_suite != a2->cipher_suite)
        && (!a1->cipher_suite || !a2->cipher_suite 
            || strcmp(a1->cipher_suite, a2->cipher_suite))) {
        return 0;
    }
    return 1;
}