int main(void)
{
  CURL *handles[HANDLECOUNT];
  CURLM *multi_handle;

  int still_running; /* keep number of running handles */
  int i;

  CURLMsg *msg; /* for picking up messages with the transfer status */
  int msgs_left; /* how many messages are left */

  /* Allocate one CURL handle per transfer */
  for(i = 0; i<HANDLECOUNT; i++)
    handles[i] = curl_easy_init();

  /* set the options (I left out a few, you'll get the point anyway) */
  curl_easy_setopt(handles[HTTP_HANDLE], CURLOPT_URL, "http://example.com");

  curl_easy_setopt(handles[FTP_HANDLE], CURLOPT_URL, "ftp://example.com");
  curl_easy_setopt(handles[FTP_HANDLE], CURLOPT_UPLOAD, 1L);

  /* init a multi stack */
  multi_handle = curl_multi_init();

  /* add the individual transfers */
  for(i = 0; i<HANDLECOUNT; i++)
    curl_multi_add_handle(multi_handle, handles[i]);

  /* we start some action by calling perform right away */
  curl_multi_perform(multi_handle, &still_running);

  do {
    struct timeval timeout;
    int rc; /* select() return code */
    CURLMcode mc; /* curl_multi_fdset() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;

    long curl_timeo = -1;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(multi_handle, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    if(mc != CURLM_OK) {
      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
      break;
    }

    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */

    if(maxfd == -1) {
#ifdef _WIN32
      Sleep(100);
      rc = 0;
#else
      /* Portable sleep for platforms other than Windows. */
      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
      rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
      /* Note that on some platforms 'timeout' may be modified by select().
         If you need access to the original value save a copy beforehand. */
      rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }

    switch(rc) {
    case -1:
      /* select error */
      break;
    case 0: /* timeout */
    default: /* action */
      curl_multi_perform(multi_handle, &still_running);
      break;
    }
  } while(still_running);

  /* See how the transfers went */
  while((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
    if(msg->msg == CURLMSG_DONE) {
      int idx, found = 0;

      /* Find out which handle this message is about */
      for(idx = 0; idx<HANDLECOUNT; idx++) {
        found = (msg->easy_handle == handles[idx]);
        if(found)
          break;
      }

      switch(idx) {
      case HTTP_HANDLE:
        printf("HTTP transfer completed with status %d\n", msg->data.result);
        break;
      case FTP_HANDLE:
        printf("FTP transfer completed with status %d\n", msg->data.result);
        break;
      }
    }
  }

  curl_multi_cleanup(multi_handle);

  /* Free the CURL handles */
  for(i = 0; i<HANDLECOUNT; i++)
    curl_easy_cleanup(handles[i]);

  return 0;
}