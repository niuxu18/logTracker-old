static CURLcode polarssl_version_from_curl(int *polarver, long ssl_version)
{
  switch(ssl_version) {
    case CURL_SSLVERSION_TLSv1_0:
      *polarver = SSL_MINOR_VERSION_1;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_1:
      *polarver = SSL_MINOR_VERSION_2;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_2:
      *polarver = SSL_MINOR_VERSION_3;
      return CURLE_OK;
    case CURL_SSLVERSION_TLSv1_3:
      break;
  }
  return CURLE_SSL_CONNECT_ERROR;
}