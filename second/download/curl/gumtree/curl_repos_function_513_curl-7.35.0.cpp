int test(char *URL)
{
  int res = 0;
  CURL *curl[NUM_HANDLES] = {0};
  int running;
  CURLM *m = NULL;
  int i;
  char target_url[256];
  char dnsentry[256];
  struct curl_slist *slist = NULL, *slist2;
  char *port = libtest_arg3;
  char *address = libtest_arg2;

  (void)URL;

  /* Create fake DNS entries for serverX.example.com for all handles */
  for(i=0; i < NUM_HANDLES; i++) {
    sprintf(dnsentry, "server%d.example.com:%s:%s", i + 1, port, address);
    printf("%s\n", dnsentry);
    slist2 = curl_slist_append(slist, dnsentry);
    if(!slist2) {
      fprintf(stderr, "curl_slist_append() failed\n");
      goto test_cleanup;
    }
    slist = slist2;
  }

  start_test_timing();

  global_init(CURL_GLOBAL_ALL);

  multi_init(m);

  multi_setopt(m, CURLMOPT_MAXCONNECTS, 3L);

  /* get NUM_HANDLES easy handles */
  for(i=0; i < NUM_HANDLES; i++) {
    /* get an easy handle */
    easy_init(curl[i]);
    /* specify target */
    sprintf(target_url, "http://server%d.example.com:%s/path/1506%04i",
            i + 1, port, i + 1);
    target_url[sizeof(target_url) - 1] = '\0';
    easy_setopt(curl[i], CURLOPT_URL, target_url);
    /* go verbose */
    easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
    /* include headers */
    easy_setopt(curl[i], CURLOPT_HEADER, 1L);

    easy_setopt(curl[i], CURLOPT_RESOLVE, slist);
  }

  fprintf(stderr, "Start at URL 0\n");

  for(i=0; i < NUM_HANDLES; i++) {
    /* add handle to multi */
    multi_add_handle(m, curl[i]);

    for(;;) {
      struct timeval interval;
      fd_set rd, wr, exc;
      int maxfd = -99;

      interval.tv_sec = 1;
      interval.tv_usec = 0;

      multi_perform(m, &running);

      abort_on_test_timeout();

      if(!running)
        break; /* done */

      FD_ZERO(&rd);
      FD_ZERO(&wr);
      FD_ZERO(&exc);

      multi_fdset(m, &rd, &wr, &exc, &maxfd);

      /* At this point, maxfd is guaranteed to be greater or equal than -1. */

      select_test(maxfd+1, &rd, &wr, &exc, &interval);

      abort_on_test_timeout();
    }
  }

test_cleanup:

  /* proper cleanup sequence - type PB */

  for(i=0; i < NUM_HANDLES; i++) {
    curl_multi_remove_handle(m, curl[i]);
    curl_easy_cleanup(curl[i]);
  }

  curl_slist_free_all(slist);

  curl_multi_cleanup(m);
  curl_global_cleanup();

  return res;
}