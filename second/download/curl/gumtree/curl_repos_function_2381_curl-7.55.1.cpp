static CURLcode darwinssl_version_from_curl(SSLProtocol *darwinver,
                                            long ssl_version)
{
  switch(ssl_version) {
    case CURL_SSLVERSION_TLSv1_0:
      *darwinver = kTLSProtocol1;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_1:
      *darwinver = kTLSProtocol11;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_2:
      *darwinver = kTLSProtocol12;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_3:
      break;
  }
  return CURLE_SSL_CONNECT_ERROR;
}