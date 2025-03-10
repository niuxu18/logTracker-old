CURLcode Curl_http_auth_act(struct connectdata *conn)
{
  struct SessionHandle *data = conn->data;
  bool pickhost = FALSE;
  bool pickproxy = FALSE;
  CURLcode code = CURLE_OK;

  if(100 == data->req.httpcode)
    /* this is a transient response code, ignore */
    return CURLE_OK;

  if(data->state.authproblem)
    return data->set.http_fail_on_error?CURLE_HTTP_RETURNED_ERROR:CURLE_OK;

  if(conn->bits.user_passwd &&
     ((data->req.httpcode == 401) ||
      (conn->bits.authneg && data->req.httpcode < 300))) {
    pickhost = pickoneauth(&data->state.authhost);
    if(!pickhost)
      data->state.authproblem = TRUE;
  }
  if(conn->bits.proxy_user_passwd &&
     ((data->req.httpcode == 407) ||
      (conn->bits.authneg && data->req.httpcode < 300))) {
    pickproxy = pickoneauth(&data->state.authproxy);
    if(!pickproxy)
      data->state.authproblem = TRUE;
  }

  if(pickhost || pickproxy) {
    data->req.newurl = strdup(data->change.url); /* clone URL */
    if(!data->req.newurl)
      return CURLE_OUT_OF_MEMORY;

    if((data->set.httpreq != HTTPREQ_GET) &&
       (data->set.httpreq != HTTPREQ_HEAD) &&
       !conn->bits.rewindaftersend) {
      code = perhapsrewind(conn);
      if(code)
        return code;
    }
  }

  else if((data->req.httpcode < 300) &&
          (!data->state.authhost.done) &&
          conn->bits.authneg) {
    /* no (known) authentication available,
       authentication is not "done" yet and
       no authentication seems to be required and
       we didn't try HEAD or GET */
    if((data->set.httpreq != HTTPREQ_GET) &&
       (data->set.httpreq != HTTPREQ_HEAD)) {
      data->req.newurl = strdup(data->change.url); /* clone URL */
      if(!data->req.newurl)
        return CURLE_OUT_OF_MEMORY;
      data->state.authhost.done = TRUE;
    }
  }
  if(Curl_http_should_fail(conn)) {
    failf (data, "The requested URL returned error: %d",
           data->req.httpcode);
    code = CURLE_HTTP_RETURNED_ERROR;
  }

  return code;
}