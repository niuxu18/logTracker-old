static unsigned int CURL_STDCALL getaddrinfo_thread (void *arg)
{
  struct thread_sync_data *tsd = (struct thread_sync_data*)arg;
  char service[12];
  int rc;

  snprintf(service, sizeof(service), "%d", tsd->port);

  rc = Curl_getaddrinfo_ex(tsd->hostname, service, &tsd->hints, &tsd->res);

  if(rc != 0) {
    tsd->sock_error = SOCKERRNO?SOCKERRNO:rc;
    if(tsd->sock_error == 0)
      tsd->sock_error = RESOLVER_ENOMEM;
  }

  Curl_mutex_acquire(tsd->mtx);
  tsd->done = 1;
  Curl_mutex_release(tsd->mtx);

  return 0;
}