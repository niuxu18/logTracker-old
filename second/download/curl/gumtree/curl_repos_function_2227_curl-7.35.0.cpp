static CURLcode
gtls_connect_step3(struct connectdata *conn,
                   int sockindex)
{
  unsigned int cert_list_size;
  const gnutls_datum *chainp;
  unsigned int verify_status;
  gnutls_x509_crt x509_cert,x509_issuer;
  gnutls_datum issuerp;
  char certbuf[256]; /* big enough? */
  size_t size;
  unsigned int algo;
  unsigned int bits;
  time_t certclock;
  const char *ptr;
  struct SessionHandle *data = conn->data;
  gnutls_session session = conn->ssl[sockindex].session;
  int rc;
  int incache;
  void *ssl_sessionid;
  CURLcode result = CURLE_OK;

  /* This function will return the peer's raw certificate (chain) as sent by
     the peer. These certificates are in raw format (DER encoded for
     X.509). In case of a X.509 then a certificate list may be present. The
     first certificate in the list is the peer's certificate, following the
     issuer's certificate, then the issuer's issuer etc. */

  chainp = gnutls_certificate_get_peers(session, &cert_list_size);
  if(!chainp) {
    if(data->set.ssl.verifypeer ||
       data->set.ssl.verifyhost ||
       data->set.ssl.issuercert) {
#ifdef USE_TLS_SRP
      if(data->set.ssl.authtype == CURL_TLSAUTH_SRP
         && data->set.ssl.username != NULL
         && !data->set.ssl.verifypeer
         && gnutls_cipher_get(session)) {
        /* no peer cert, but auth is ok if we have SRP user and cipher and no
           peer verify */
      }
      else {
#endif
        failf(data, "failed to get server cert");
        return CURLE_PEER_FAILED_VERIFICATION;
#ifdef USE_TLS_SRP
      }
#endif
    }
    infof(data, "\t common name: WARNING couldn't obtain\n");
  }

  if(data->set.ssl.verifypeer) {
    /* This function will try to verify the peer's certificate and return its
       status (trusted, invalid etc.). The value of status should be one or
       more of the gnutls_certificate_status_t enumerated elements bitwise
       or'd. To avoid denial of service attacks some default upper limits
       regarding the certificate key size and chain size are set. To override
       them use gnutls_certificate_set_verify_limits(). */

    rc = gnutls_certificate_verify_peers2(session, &verify_status);
    if(rc < 0) {
      failf(data, "server cert verify failed: %d", rc);
      return CURLE_SSL_CONNECT_ERROR;
    }

    /* verify_status is a bitmask of gnutls_certificate_status bits */
    if(verify_status & GNUTLS_CERT_INVALID) {
      if(data->set.ssl.verifypeer) {
        failf(data, "server certificate verification failed. CAfile: %s "
              "CRLfile: %s", data->set.ssl.CAfile?data->set.ssl.CAfile:"none",
              data->set.ssl.CRLfile?data->set.ssl.CRLfile:"none");
        return CURLE_SSL_CACERT;
      }
      else
        infof(data, "\t server certificate verification FAILED\n");
    }
    else
      infof(data, "\t server certificate verification OK\n");
  }
  else
    infof(data, "\t server certificate verification SKIPPED\n");

  /* initialize an X.509 certificate structure. */
  gnutls_x509_crt_init(&x509_cert);

  /* convert the given DER or PEM encoded Certificate to the native
     gnutls_x509_crt_t format */
  gnutls_x509_crt_import(x509_cert, chainp, GNUTLS_X509_FMT_DER);

  if(data->set.ssl.issuercert) {
    gnutls_x509_crt_init(&x509_issuer);
    issuerp = load_file(data->set.ssl.issuercert);
    gnutls_x509_crt_import(x509_issuer, &issuerp, GNUTLS_X509_FMT_PEM);
    rc = gnutls_x509_crt_check_issuer(x509_cert,x509_issuer);
    unload_file(issuerp);
    if(rc <= 0) {
      failf(data, "server certificate issuer check failed (IssuerCert: %s)",
            data->set.ssl.issuercert?data->set.ssl.issuercert:"none");
      return CURLE_SSL_ISSUER_ERROR;
    }
    infof(data,"\t server certificate issuer check OK (Issuer Cert: %s)\n",
          data->set.ssl.issuercert?data->set.ssl.issuercert:"none");
  }

  size=sizeof(certbuf);
  rc = gnutls_x509_crt_get_dn_by_oid(x509_cert, GNUTLS_OID_X520_COMMON_NAME,
                                     0, /* the first and only one */
                                     FALSE,
                                     certbuf,
                                     &size);
  if(rc) {
    infof(data, "error fetching CN from cert:%s\n",
          gnutls_strerror(rc));
  }

  /* This function will check if the given certificate's subject matches the
     given hostname. This is a basic implementation of the matching described
     in RFC2818 (HTTPS), which takes into account wildcards, and the subject
     alternative name PKIX extension. Returns non zero on success, and zero on
     failure. */
  rc = gnutls_x509_crt_check_hostname(x509_cert, conn->host.name);

  if(!rc) {
    if(data->set.ssl.verifyhost) {
      failf(data, "SSL: certificate subject name (%s) does not match "
            "target host name '%s'", certbuf, conn->host.dispname);
      gnutls_x509_crt_deinit(x509_cert);
      return CURLE_PEER_FAILED_VERIFICATION;
    }
    else
      infof(data, "\t common name: %s (does not match '%s')\n",
            certbuf, conn->host.dispname);
  }
  else
    infof(data, "\t common name: %s (matched)\n", certbuf);

  /* Check for time-based validity */
  certclock = gnutls_x509_crt_get_expiration_time(x509_cert);

  if(certclock == (time_t)-1) {
    failf(data, "server cert expiration date verify failed");
    return CURLE_SSL_CONNECT_ERROR;
  }

  if(certclock < time(NULL)) {
    if(data->set.ssl.verifypeer) {
      failf(data, "server certificate expiration date has passed.");
      return CURLE_PEER_FAILED_VERIFICATION;
    }
    else
      infof(data, "\t server certificate expiration date FAILED\n");
  }
  else
    infof(data, "\t server certificate expiration date OK\n");

  certclock = gnutls_x509_crt_get_activation_time(x509_cert);

  if(certclock == (time_t)-1) {
    failf(data, "server cert activation date verify failed");
    return CURLE_SSL_CONNECT_ERROR;
  }

  if(certclock > time(NULL)) {
    if(data->set.ssl.verifypeer) {
      failf(data, "server certificate not activated yet.");
      return CURLE_PEER_FAILED_VERIFICATION;
    }
    else
      infof(data, "\t server certificate activation date FAILED\n");
  }
  else
    infof(data, "\t server certificate activation date OK\n");

  /* Show:

  - ciphers used
  - subject
  - start date
  - expire date
  - common name
  - issuer

  */

  /* public key algorithm's parameters */
  algo = gnutls_x509_crt_get_pk_algorithm(x509_cert, &bits);
  infof(data, "\t certificate public key: %s\n",
        gnutls_pk_algorithm_get_name(algo));

  /* version of the X.509 certificate. */
  infof(data, "\t certificate version: #%d\n",
        gnutls_x509_crt_get_version(x509_cert));


  size = sizeof(certbuf);
  gnutls_x509_crt_get_dn(x509_cert, certbuf, &size);
  infof(data, "\t subject: %s\n", certbuf);

  certclock = gnutls_x509_crt_get_activation_time(x509_cert);
  showtime(data, "start date", certclock);

  certclock = gnutls_x509_crt_get_expiration_time(x509_cert);
  showtime(data, "expire date", certclock);

  size = sizeof(certbuf);
  gnutls_x509_crt_get_issuer_dn(x509_cert, certbuf, &size);
  infof(data, "\t issuer: %s\n", certbuf);

  gnutls_x509_crt_deinit(x509_cert);

  /* compression algorithm (if any) */
  ptr = gnutls_compression_get_name(gnutls_compression_get(session));
  /* the *_get_name() says "NULL" if GNUTLS_COMP_NULL is returned */
  infof(data, "\t compression: %s\n", ptr);

  /* the name of the cipher used. ie 3DES. */
  ptr = gnutls_cipher_get_name(gnutls_cipher_get(session));
  infof(data, "\t cipher: %s\n", ptr);

  /* the MAC algorithms name. ie SHA1 */
  ptr = gnutls_mac_get_name(gnutls_mac_get(session));
  infof(data, "\t MAC: %s\n", ptr);

  conn->ssl[sockindex].state = ssl_connection_complete;
  conn->recv[sockindex] = gtls_recv;
  conn->send[sockindex] = gtls_send;

  {
    /* we always unconditionally get the session id here, as even if we
       already got it from the cache and asked to use it in the connection, it
       might've been rejected and then a new one is in use now and we need to
       detect that. */
    void *connect_sessionid;
    size_t connect_idsize;

    /* get the session ID data size */
    gnutls_session_get_data(session, NULL, &connect_idsize);
    connect_sessionid = malloc(connect_idsize); /* get a buffer for it */

    if(connect_sessionid) {
      /* extract session ID to the allocated buffer */
      gnutls_session_get_data(session, connect_sessionid, &connect_idsize);

      incache = !(Curl_ssl_getsessionid(conn, &ssl_sessionid, NULL));
      if(incache) {
        /* there was one before in the cache, so instead of risking that the
           previous one was rejected, we just kill that and store the new */
        Curl_ssl_delsessionid(conn, ssl_sessionid);
      }

      /* store this session id */
      result = Curl_ssl_addsessionid(conn, connect_sessionid, connect_idsize);
      if(result) {
        free(connect_sessionid);
        result = CURLE_OUT_OF_MEMORY;
      }
    }
    else
      result = CURLE_OUT_OF_MEMORY;
  }

  return result;
}