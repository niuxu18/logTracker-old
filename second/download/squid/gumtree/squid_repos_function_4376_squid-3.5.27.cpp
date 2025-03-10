static BIGNUM *x509Digest(Ssl::X509_Pointer const & cert)
{
    unsigned int n;
    unsigned char md[EVP_MAX_MD_SIZE];

    if (!X509_digest(cert.get(),EVP_sha1(),md,&n))
        return NULL;

    return createCertSerial(md, n);
}