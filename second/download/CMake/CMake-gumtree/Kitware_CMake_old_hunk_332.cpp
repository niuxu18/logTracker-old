  }
#endif

#ifdef USE_LIBIDN
  idna_init();
#endif

  if(Curl_resolver_global_init()) {
    DEBUGF(fprintf(stderr, "Error: resolver_global_init failed\n"));
    return CURLE_FAILED_INIT;
