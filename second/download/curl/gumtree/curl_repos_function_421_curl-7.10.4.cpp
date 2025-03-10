CURL *curl_easy_duphandle(CURL *incurl)
{
  struct SessionHandle *data=(struct SessionHandle *)incurl;

  struct SessionHandle *outcurl = (struct SessionHandle *)
    malloc(sizeof(struct SessionHandle));

  if(NULL == outcurl)
    return NULL; /* failure */

  /* start with clearing the entire new struct */
  memset(outcurl, 0, sizeof(struct SessionHandle));

  /*
   * We setup a few buffers we need. We should probably make them
   * get setup on-demand in the code, as that would probably decrease
   * the likeliness of us forgetting to init a buffer here in the future.
   */
  outcurl->state.headerbuff=(char*)malloc(HEADERSIZE);
  if(!outcurl->state.headerbuff) {
    free(outcurl); /* free the memory again */
    return NULL;
  }
  outcurl->state.headersize=HEADERSIZE;

  /* copy all userdefined values */
  outcurl->set = data->set;
  outcurl->state.numconnects = data->state.numconnects;
  outcurl->state.connects = (struct connectdata **)
      malloc(sizeof(struct connectdata *) * outcurl->state.numconnects);

  if(!outcurl->state.connects) {
    free(outcurl->state.headerbuff);
    free(outcurl);
    return NULL;
  }
  memset(outcurl->state.connects, 0,
         sizeof(struct connectdata *)*outcurl->state.numconnects);

  outcurl->progress.flags    = data->progress.flags;
  outcurl->progress.callback = data->progress.callback;

  if(data->cookies)
    /* If cookies are enabled in the parent handle, we enable them
       in the clone as well! */
    outcurl->cookies = Curl_cookie_init(data->cookies->filename,
                                        outcurl->cookies,
                                        data->set.cookiesession);

  /* duplicate all values in 'change' */
  if(data->change.url) {
    outcurl->change.url = strdup(data->change.url);
    outcurl->change.url_alloc = TRUE;
  }
  if(data->change.proxy) {
    outcurl->change.proxy = strdup(data->change.proxy);
    outcurl->change.proxy_alloc = TRUE;
  }
  if(data->change.referer) {
    outcurl->change.referer = strdup(data->change.referer);
    outcurl->change.referer_alloc = TRUE;
  }

  return outcurl;
}