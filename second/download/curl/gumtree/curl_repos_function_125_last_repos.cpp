int main(void)
{
  CURL *curl;
  CURLM *mcurl;
  int still_running = 1;
  struct timeval mp_start;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;

  upload_ctx.lines_read = 0;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(!curl)
    return 1;

  mcurl = curl_multi_init();
  if(!mcurl)
    return 2;

  /* This is the URL for your mailserver */
  curl_easy_setopt(curl, CURLOPT_URL, "smtp://mail.example.com");

  /* Note that this option isn't strictly required, omitting it will result in
   * libcurl sending the MAIL FROM command with empty sender data. All
   * autoresponses should have an empty reverse-path, and should be directed
   * to the address in the reverse-path which triggered them. Otherwise, they
   * could cause an endless loop. See RFC 5321 Section 4.5.5 for more details.
   */
  curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

  /* Add two recipients, in this particular case they correspond to the
   * To: and Cc: addressees in the header, but they could be any kind of
   * recipient. */
  recipients = curl_slist_append(recipients, TO);
  recipients = curl_slist_append(recipients, CC);
  curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

  /* We're using a callback function to specify the payload (the headers and
   * body of the message). You could just use the CURLOPT_READDATA option to
   * specify a FILE pointer to read from. */
  curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
  curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

  /* Tell the multi stack about our easy handle */
  curl_multi_add_handle(mcurl, curl);

  /* Record the start time which we can use later */
  mp_start = tvnow();

  /* We start some action by calling perform right away */
  curl_multi_perform(mcurl, &still_running);

  while(still_running) {
    struct timeval timeout;
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;
    int rc;
    CURLMcode mc; /* curl_multi_fdset() return code */

    long curl_timeo = -1;

    /* Initialise the file descriptors */
    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* Set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(mcurl, &curl_timeo);
    if(curl_timeo >= 0) {
      timeout.tv_sec = curl_timeo / 1000;
      if(timeout.tv_sec > 1)
        timeout.tv_sec = 1;
      else
        timeout.tv_usec = (curl_timeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(mcurl, &fdread, &fdwrite, &fdexcep, &maxfd);

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

    if(tvdiff(tvnow(), mp_start) > MULTI_PERFORM_HANG_TIMEOUT) {
      fprintf(stderr,
              "ABORTING: Since it seems that we would have run forever.\n");
      break;
    }

    switch(rc) {
    case -1:  /* select error */
      break;
    case 0:   /* timeout */
    default:  /* action */
      curl_multi_perform(mcurl, &still_running);
      break;
    }
  }

  /* Free the list of recipients */
  curl_slist_free_all(recipients);

  /* Always cleanup */
  curl_multi_remove_handle(mcurl, curl);
  curl_multi_cleanup(mcurl);
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return 0;
}