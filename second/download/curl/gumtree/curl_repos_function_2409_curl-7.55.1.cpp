static CURLcode
cyassl_connect_step1(struct connectdata *conn,
                     int sockindex)
{
  char error_buffer[CYASSL_MAX_ERROR_SZ];
  char *ciphers;
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data* conssl = &conn->ssl[sockindex];
  SSL_METHOD* req_method = NULL;
  curl_socket_t sockfd = conn->sock[sockindex];
#ifdef HAVE_SNI
  bool sni = FALSE;
#define use_sni(x)  sni = (x)
#else
#define use_sni(x)  Curl_nop_stmt
#endif

  if(conssl->state == ssl_connection_complete)
    return CURLE_OK;

  if(SSL_CONN_CONFIG(version_max) != CURL_SSLVERSION_MAX_NONE) {
    failf(data, "CyaSSL does not support to set maximum SSL/TLS version");
    return CURLE_SSL_CONNECT_ERROR;
  }

  /* check to see if we've been told to use an explicit SSL/TLS version */
  switch(SSL_CONN_CONFIG(version)) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
#if LIBCYASSL_VERSION_HEX >= 0x03003000 /* >= 3.3.0 */
    /* minimum protocol version is set later after the CTX object is created */
    req_method = SSLv23_client_method();
#else
    infof(data, "CyaSSL <3.3.0 cannot be configured to use TLS 1.0-1.2, "
          "TLS 1.0 is used exclusively\n");
    req_method = TLSv1_client_method();
#endif
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_TLSv1_0:
    req_method = TLSv1_client_method();
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_TLSv1_1:
    req_method = TLSv1_1_client_method();
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_TLSv1_2:
    req_method = TLSv1_2_client_method();
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_TLSv1_3:
    failf(data, "CyaSSL: TLS 1.3 is not yet supported");
    return CURLE_SSL_CONNECT_ERROR;
  case CURL_SSLVERSION_SSLv3:
#ifdef WOLFSSL_ALLOW_SSLV3
    req_method = SSLv3_client_method();
    use_sni(FALSE);
#else
    failf(data, "CyaSSL does not support SSLv3");
    return CURLE_NOT_BUILT_IN;
#endif
    break;
  case CURL_SSLVERSION_SSLv2:
    failf(data, "CyaSSL does not support SSLv2");
    return CURLE_SSL_CONNECT_ERROR;
  default:
    failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
    return CURLE_SSL_CONNECT_ERROR;
  }

  if(!req_method) {
    failf(data, "SSL: couldn't create a method!");
    return CURLE_OUT_OF_MEMORY;
  }

  if(conssl->ctx)
    SSL_CTX_free(conssl->ctx);
  conssl->ctx = SSL_CTX_new(req_method);

  if(!conssl->ctx) {
    failf(data, "SSL: couldn't create a context!");
    return CURLE_OUT_OF_MEMORY;
  }

  switch(SSL_CONN_CONFIG(version)) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
#if LIBCYASSL_VERSION_HEX > 0x03004006 /* > 3.4.6 */
    /* Versions 3.3.0 to 3.4.6 we know the minimum protocol version is whatever
    minimum version of TLS was built in and at least TLS 1.0. For later library
    versions that could change (eg TLS 1.0 built in but defaults to TLS 1.1) so
    we have this short circuit evaluation to find the minimum supported TLS
    version. We use wolfSSL_CTX_SetMinVersion and not CyaSSL_SetMinVersion
    because only the former will work before the user's CTX callback is called.
    */
    if((wolfSSL_CTX_SetMinVersion(conssl->ctx, WOLFSSL_TLSV1) != 1) &&
       (wolfSSL_CTX_SetMinVersion(conssl->ctx, WOLFSSL_TLSV1_1) != 1) &&
       (wolfSSL_CTX_SetMinVersion(conssl->ctx, WOLFSSL_TLSV1_2) != 1)) {
      failf(data, "SSL: couldn't set the minimum protocol version");
      return CURLE_SSL_CONNECT_ERROR;
    }
#endif
    break;
  }

  ciphers = SSL_CONN_CONFIG(cipher_list);
  if(ciphers) {
    if(!SSL_CTX_set_cipher_list(conssl->ctx, ciphers)) {
      failf(data, "failed setting cipher list: %s", ciphers);
      return CURLE_SSL_CIPHER;
    }
    infof(data, "Cipher selection: %s\n", ciphers);
  }

#ifndef NO_FILESYSTEM
  /* load trusted cacert */
  if(SSL_CONN_CONFIG(CAfile)) {
    if(1 != SSL_CTX_load_verify_locations(conssl->ctx,
                                      SSL_CONN_CONFIG(CAfile),
                                      SSL_CONN_CONFIG(CApath))) {
      if(SSL_CONN_CONFIG(verifypeer)) {
        /* Fail if we insist on successfully verifying the server. */
        failf(data, "error setting certificate verify locations:\n"
              "  CAfile: %s\n  CApath: %s",
              SSL_CONN_CONFIG(CAfile)?
              SSL_CONN_CONFIG(CAfile): "none",
              SSL_CONN_CONFIG(CApath)?
              SSL_CONN_CONFIG(CApath) : "none");
        return CURLE_SSL_CACERT_BADFILE;
      }
      else {
        /* Just continue with a warning if no strict certificate
           verification is required. */
        infof(data, "error setting certificate verify locations,"
              " continuing anyway:\n");
      }
    }
    else {
      /* Everything is fine. */
      infof(data, "successfully set certificate verify locations:\n");
    }
    infof(data,
          "  CAfile: %s\n"
          "  CApath: %s\n",
          SSL_CONN_CONFIG(CAfile) ? SSL_CONN_CONFIG(CAfile):
          "none",
          SSL_CONN_CONFIG(CApath) ? SSL_CONN_CONFIG(CApath):
          "none");
  }

  /* Load the client certificate, and private key */
  if(SSL_SET_OPTION(cert) && SSL_SET_OPTION(key)) {
    int file_type = do_file_type(SSL_SET_OPTION(cert_type));

    if(SSL_CTX_use_certificate_file(conssl->ctx, SSL_SET_OPTION(cert),
                                     file_type) != 1) {
      failf(data, "unable to use client certificate (no key or wrong pass"
            " phrase?)");
      return CURLE_SSL_CONNECT_ERROR;
    }

    file_type = do_file_type(SSL_SET_OPTION(key_type));
    if(SSL_CTX_use_PrivateKey_file(conssl->ctx, SSL_SET_OPTION(key),
                                    file_type) != 1) {
      failf(data, "unable to set private key");
      return CURLE_SSL_CONNECT_ERROR;
    }
  }
#endif /* !NO_FILESYSTEM */

  /* SSL always tries to verify the peer, this only says whether it should
   * fail to connect if the verification fails, or if it should continue
   * anyway. In the latter case the result of the verification is checked with
   * SSL_get_verify_result() below. */
  SSL_CTX_set_verify(conssl->ctx,
                     SSL_CONN_CONFIG(verifypeer)?SSL_VERIFY_PEER:
                                                 SSL_VERIFY_NONE,
                     NULL);

#ifdef HAVE_SNI
  if(sni) {
    struct in_addr addr4;
#ifdef ENABLE_IPV6
    struct in6_addr addr6;
#endif
    const char * const hostname = SSL_IS_PROXY() ? conn->http_proxy.host.name :
      conn->host.name;
    size_t hostname_len = strlen(hostname);
    if((hostname_len < USHRT_MAX) &&
       (0 == Curl_inet_pton(AF_INET, hostname, &addr4)) &&
#ifdef ENABLE_IPV6
       (0 == Curl_inet_pton(AF_INET6, hostname, &addr6)) &&
#endif
       (CyaSSL_CTX_UseSNI(conssl->ctx, CYASSL_SNI_HOST_NAME, hostname,
                          (unsigned short)hostname_len) != 1)) {
      infof(data, "WARNING: failed to configure server name indication (SNI) "
            "TLS extension\n");
    }
  }
#endif

#ifdef HAVE_SUPPORTED_CURVES
  /* CyaSSL/wolfSSL does not send the supported ECC curves ext automatically:
     https://github.com/wolfSSL/wolfssl/issues/366
     The supported curves below are those also supported by OpenSSL 1.0.2 and
     in the same order. */
  CyaSSL_CTX_UseSupportedCurve(conssl->ctx, 0x17); /* secp256r1 */
  CyaSSL_CTX_UseSupportedCurve(conssl->ctx, 0x19); /* secp521r1 */
  CyaSSL_CTX_UseSupportedCurve(conssl->ctx, 0x18); /* secp384r1 */
#endif

  /* give application a chance to interfere with SSL set up. */
  if(data->set.ssl.fsslctx) {
    CURLcode result = CURLE_OK;
    result = (*data->set.ssl.fsslctx)(data, conssl->ctx,
                                      data->set.ssl.fsslctxp);
    if(result) {
      failf(data, "error signaled by ssl ctx callback");
      return result;
    }
  }
#ifdef NO_FILESYSTEM
  else if(SSL_CONN_CONFIG(verifypeer)) {
    failf(data, "SSL: Certificates couldn't be loaded because CyaSSL was built"
          " with \"no filesystem\". Either disable peer verification"
          " (insecure) or if you are building an application with libcurl you"
          " can load certificates via CURLOPT_SSL_CTX_FUNCTION.");
    return CURLE_SSL_CONNECT_ERROR;
  }
#endif

  /* Let's make an SSL structure */
  if(conssl->handle)
    SSL_free(conssl->handle);
  conssl->handle = SSL_new(conssl->ctx);
  if(!conssl->handle) {
    failf(data, "SSL: couldn't create a context (handle)!");
    return CURLE_OUT_OF_MEMORY;
  }

#ifdef HAVE_ALPN
  if(conn->bits.tls_enable_alpn) {
    char protocols[128];
    *protocols = '\0';

    /* wolfSSL's ALPN protocol name list format is a comma separated string of
       protocols in descending order of preference, eg: "h2,http/1.1" */

#ifdef USE_NGHTTP2
    if(data->set.httpversion >= CURL_HTTP_VERSION_2) {
      strcpy(protocols + strlen(protocols), NGHTTP2_PROTO_VERSION_ID ",");
      infof(data, "ALPN, offering %s\n", NGHTTP2_PROTO_VERSION_ID);
    }
#endif

    strcpy(protocols + strlen(protocols), ALPN_HTTP_1_1);
    infof(data, "ALPN, offering %s\n", ALPN_HTTP_1_1);

    if(wolfSSL_UseALPN(conssl->handle, protocols,
                       (unsigned)strlen(protocols),
                       WOLFSSL_ALPN_CONTINUE_ON_MISMATCH) != SSL_SUCCESS) {
      failf(data, "SSL: failed setting ALPN protocols");
      return CURLE_SSL_CONNECT_ERROR;
    }
  }
#endif /* HAVE_ALPN */

  /* Check if there's a cached ID we can/should use here! */
  if(SSL_SET_OPTION(primary.sessionid)) {
    void *ssl_sessionid = NULL;

    Curl_ssl_sessionid_lock(conn);
    if(!Curl_ssl_getsessionid(conn, &ssl_sessionid, NULL, sockindex)) {
      /* we got a session id, use it! */
      if(!SSL_set_session(conssl->handle, ssl_sessionid)) {
        Curl_ssl_sessionid_unlock(conn);
        failf(data, "SSL: SSL_set_session failed: %s",
              ERR_error_string(SSL_get_error(conssl->handle, 0),
              error_buffer));
        return CURLE_SSL_CONNECT_ERROR;
      }
      /* Informational message */
      infof(data, "SSL re-using session ID\n");
    }
    Curl_ssl_sessionid_unlock(conn);
  }

  /* pass the raw socket into the SSL layer */
  if(!SSL_set_fd(conssl->handle, (int)sockfd)) {
    failf(data, "SSL: SSL_set_fd failed");
    return CURLE_SSL_CONNECT_ERROR;
  }

  conssl->connecting_state = ssl_connect_2;
  return CURLE_OK;
}