int test(char *URL)
{
  CURL *curl;
  CURLcode res=CURLE_OK;
  struct curl_slist *slist = NULL;

  struct WriteThis pooh;
  pooh.counter = 0;

  if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
    fprintf(stderr, "curl_global_init() failed\n");
    return TEST_ERR_MAJOR_BAD;
  }

  if ((curl = curl_easy_init()) == NULL) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  slist = curl_slist_append(slist, "Transfer-Encoding: chunked");
  if (slist == NULL) {
    fprintf(stderr, "curl_slist_append() failed\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  /* First set the URL that is about to receive our POST. */
  curl_easy_setopt(curl, CURLOPT_URL, URL);

  /* Now specify we want to POST data */
  curl_easy_setopt(curl, CURLOPT_POST, TRUE);

  /* we want to use our own read function */
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

  /* pointer to pass to our read function */
  curl_easy_setopt(curl, CURLOPT_INFILE, &pooh);

  /* get verbose debug output please */
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

  /* include headers in the output */
  curl_easy_setopt(curl, CURLOPT_HEADER, TRUE);

  /* enforce chunked transfer by setting the header */
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

  /* Perform the request, res will get the return code */
  res = curl_easy_perform(curl);

  /* clean up the headers list */
  if(slist)
    curl_slist_free_all(slist);

  /* always cleanup */
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return res;
}