int test(char *URL)
{
  int res = 0;
  CURL *curl[NUM_HANDLES] = {NULL, NULL};
  char *port = libtest_arg3;
  char *address = libtest_arg2;
  char dnsentry[256];
  struct curl_slist *slist = NULL;
  int i;
  char target_url[256];
  (void)URL; /* URL is setup in the code */

  if(curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  snprintf(dnsentry, sizeof(dnsentry), "server.example.curl:%s:%s",
           port, address);
  printf("%s\n", dnsentry);
  slist = curl_slist_append(slist, dnsentry);

  /* get NUM_HANDLES easy handles */
  for(i = 0; i < NUM_HANDLES; i++) {
    /* get an easy handle */
    easy_init(curl[i]);
    /* specify target */
    snprintf(target_url, sizeof(target_url),
             "http://server.example.curl:%s/path/1512%04i",
             port, i + 1);
    target_url[sizeof(target_url) - 1] = '\0';
    easy_setopt(curl[i], CURLOPT_URL, target_url);
    /* go verbose */
    easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
    /* include headers */
    easy_setopt(curl[i], CURLOPT_HEADER, 1L);

    easy_setopt(curl[i], CURLOPT_DNS_USE_GLOBAL_CACHE, 1L);
  }

  /* make the first one populate the GLOBAL cache */
  easy_setopt(curl[0], CURLOPT_RESOLVE, slist);

  /* run NUM_HANDLES transfers */
  for(i = 0; (i < NUM_HANDLES) && !res; i++)
    res = curl_easy_perform(curl[i]);

test_cleanup:

  curl_easy_cleanup(curl[0]);
  curl_easy_cleanup(curl[1]);
  curl_slist_free_all(slist);
  curl_global_cleanup();

  return res;
}