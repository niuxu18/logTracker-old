int main(void)
{
  CURL *ch;
  CURLcode rv;

  rv = curl_global_init(CURL_GLOBAL_ALL);
  ch = curl_easy_init();
  rv = curl_easy_setopt(ch,CURLOPT_VERBOSE, 0L);
  rv = curl_easy_setopt(ch,CURLOPT_HEADER, 0L);
  rv = curl_easy_setopt(ch,CURLOPT_NOPROGRESS, 1L);
  rv = curl_easy_setopt(ch,CURLOPT_NOSIGNAL, 1L);
  rv = curl_easy_setopt(ch,CURLOPT_WRITEFUNCTION, *writefunction);
  rv = curl_easy_setopt(ch,CURLOPT_WRITEDATA, stdout);
  rv = curl_easy_setopt(ch,CURLOPT_HEADERFUNCTION, *writefunction);
  rv = curl_easy_setopt(ch,CURLOPT_WRITEHEADER, stderr);
  rv = curl_easy_setopt(ch,CURLOPT_SSLCERTTYPE,"PEM");

  /* both VERIFYPEER and VERIFYHOST are set to 0 in this case because there is
     no CA certificate*/

  rv = curl_easy_setopt(ch,CURLOPT_SSL_VERIFYPEER, 0L);
  rv = curl_easy_setopt(ch,CURLOPT_SSL_VERIFYHOST, 0L);
  rv = curl_easy_setopt(ch, CURLOPT_URL, "https://www.example.com/");
  rv = curl_easy_setopt(ch, CURLOPT_SSLKEYTYPE, "PEM");

  /* first try: retrieve page without user certificate and key -> will fail
   */
  rv = curl_easy_perform(ch);
  if (rv==CURLE_OK) {
    printf("*** transfer succeeded ***\n");
  }
  else {
    printf("*** transfer failed ***\n");
  }

  /* second try: retrieve page using user certificate and key -> will succeed
   * load the certificate and key by installing a function doing the necessary
   * "modifications" to the SSL CONTEXT just before link init
   */
  rv = curl_easy_setopt(ch,CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
  rv = curl_easy_perform(ch);
  if (rv==CURLE_OK) {
    printf("*** transfer succeeded ***\n");
  }
  else {
    printf("*** transfer failed ***\n");
  }

  curl_easy_cleanup(ch);
  curl_global_cleanup();
  return rv;
}