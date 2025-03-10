CURLcode Curl_pretransfer(struct Curl_easy *data)
{
  CURLcode result;
  if(!data->change.url) {
    /* we can't do anything without URL */
    failf(data, "No URL set!");
    return CURLE_URL_MALFORMAT;
  }

  /* Init the SSL session ID cache here. We do it here since we want to do it
     after the *_setopt() calls (that could specify the size of the cache) but
     before any transfer takes place. */
  result = Curl_ssl_initsessions(data, data->set.ssl.max_ssl_sessions);
  if(result)
    return result;

  data->set.followlocation=0; /* reset the location-follow counter */
  data->state.this_is_a_follow = FALSE; /* reset this */
  data->state.errorbuf = FALSE; /* no error has occurred */
  data->state.httpversion = 0; /* don't assume any particular server version */

  data->state.authproblem = FALSE;
  data->state.authhost.want = data->set.httpauth;
  data->state.authproxy.want = data->set.proxyauth;
  Curl_safefree(data->info.wouldredirect);
  data->info.wouldredirect = NULL;

  if(data->set.httpreq == HTTPREQ_PUT)
    data->state.infilesize = data->set.filesize;
  else
    data->state.infilesize = data->set.postfieldsize;

  /* If there is a list of cookie files to read, do it now! */
  if(data->change.cookielist)
    Curl_cookie_loadfiles(data);

  /* If there is a list of host pairs to deal with */
  if(data->change.resolve)
    result = Curl_loadhostpairs(data);

  if(!result) {
    /* Allow data->set.use_port to set which port to use. This needs to be
     * disabled for example when we follow Location: headers to URLs using
     * different ports! */
    data->state.allow_port = TRUE;

#if defined(HAVE_SIGNAL) && defined(SIGPIPE) && !defined(HAVE_MSG_NOSIGNAL)
    /*************************************************************
     * Tell signal handler to ignore SIGPIPE
     *************************************************************/
    if(!data->set.no_signal)
      data->state.prev_signal = signal(SIGPIPE, SIG_IGN);
#endif

    Curl_initinfo(data); /* reset session-specific information "variables" */
    Curl_pgrsResetTimesSizes(data);
    Curl_pgrsStartNow(data);

    if(data->set.timeout)
      Curl_expire(data, data->set.timeout);

    if(data->set.connecttimeout)
      Curl_expire(data, data->set.connecttimeout);

    /* In case the handle is re-used and an authentication method was picked
       in the session we need to make sure we only use the one(s) we now
       consider to be fine */
    data->state.authhost.picked &= data->state.authhost.want;
    data->state.authproxy.picked &= data->state.authproxy.want;

    if(data->set.wildcardmatch) {
      struct WildcardData *wc = &data->wildcard;
      if(!wc->filelist) {
        result = Curl_wildcard_init(wc); /* init wildcard structures */
        if(result)
          return CURLE_OUT_OF_MEMORY;
      }
    }

  }

  return result;
}