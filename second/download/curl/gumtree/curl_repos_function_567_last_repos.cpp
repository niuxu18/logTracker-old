int test(char *URL)
{
  int res = 0;
  CURL *curl[NUM_HANDLES];
  int running;
  CURLM *m = NULL;
  int i;
  char target_url[256];
  int handles_added = 0;

  for(i = 0; i < NUM_HANDLES; i++)
    curl[i] = NULL;

  start_test_timing();

  global_init(CURL_GLOBAL_ALL);

  multi_init(m);

  /* get NUM_HANDLES easy handles */
  for(i = 0; i < NUM_HANDLES; i++) {
    /* get an easy handle */
    easy_init(curl[i]);
    /* specify target */
    snprintf(target_url, sizeof(target_url), "%s%04i", URL, i + 1);
    target_url[sizeof(target_url) - 1] = '\0';
    easy_setopt(curl[i], CURLOPT_URL, target_url);
    /* go verbose */
    easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
    /* include headers */
    easy_setopt(curl[i], CURLOPT_HEADER, 1L);
  }

  /* Add the first handle to multi. We do this to let libcurl detect
     that the server can do pipelining. The rest of the handles will be
     added later. */
  multi_add_handle(m, curl[handles_added++]);

  multi_setopt(m, CURLMOPT_PIPELINING, 1L);

  fprintf(stderr, "Start at URL 0\n");

  for(;;) {
    struct timeval interval;
    fd_set rd, wr, exc;
    int maxfd = -99;

    interval.tv_sec = 1;
    interval.tv_usec = 0;

    multi_perform(m, &running);

    abort_on_test_timeout();

    if(!running) {
      if(handles_added >= NUM_HANDLES)
        break; /* done */

      /* Add the rest of the handles now that the first handle has completed
         its request. */
      while(handles_added < NUM_HANDLES)
        multi_add_handle(m, curl[handles_added++]);
    }

    FD_ZERO(&rd);
    FD_ZERO(&wr);
    FD_ZERO(&exc);

    multi_fdset(m, &rd, &wr, &exc, &maxfd);

    /* At this point, maxfd is guaranteed to be greater or equal than -1. */

    select_test(maxfd + 1, &rd, &wr, &exc, &interval);

    abort_on_test_timeout();
  }

test_cleanup:

  /* proper cleanup sequence - type PB */

  for(i = 0; i < NUM_HANDLES; i++) {
    curl_multi_remove_handle(m, curl[i]);
    curl_easy_cleanup(curl[i]);
  }

  curl_multi_cleanup(m);
  curl_global_cleanup();

  return res;
}