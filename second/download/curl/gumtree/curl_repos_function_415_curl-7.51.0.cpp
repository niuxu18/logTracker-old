int test(char *URL)
{
  CURLcode res;
  CURL *curl;

  if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  if((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  test_setopt(curl, CURLOPT_URL, URL);
  test_setopt(curl, CURLOPT_HEADER, 1L);
  test_setopt(curl, CURLOPT_PROXYAUTH,
              (long) (CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NTLM));
  test_setopt(curl, CURLOPT_PROXY, libtest_arg2); /* set in first.c */
  test_setopt(curl, CURLOPT_PROXYUSERPWD, "me:password");

  res = curl_easy_perform(curl);

  test_cleanup:

  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return (int)res;
}