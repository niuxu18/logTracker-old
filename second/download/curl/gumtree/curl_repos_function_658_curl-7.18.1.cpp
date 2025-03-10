int Curl_http_should_fail(struct connectdata *conn)
{
  struct SessionHandle *data;
  int httpcode;

  DEBUGASSERT(conn);
  data = conn->data;
  DEBUGASSERT(data);

  httpcode = data->req.httpcode;

  /*
  ** If we haven't been asked to fail on error,
  ** don't fail.
  */
  if(!data->set.http_fail_on_error)
    return 0;

  /*
  ** Any code < 400 is never terminal.
  */
  if(httpcode < 400)
    return 0;

  if(data->state.resume_from &&
     (data->set.httpreq==HTTPREQ_GET) &&
     (httpcode == 416)) {
    /* "Requested Range Not Satisfiable", just proceed and
       pretend this is no error */
    return 0;
  }

  /*
  ** Any code >= 400 that's not 401 or 407 is always
  ** a terminal error
  */
  if((httpcode != 401) &&
      (httpcode != 407))
    return 1;

  /*
  ** All we have left to deal with is 401 and 407
  */
  DEBUGASSERT((httpcode == 401) || (httpcode == 407));

  /*
  ** Examine the current authentication state to see if this
  ** is an error.  The idea is for this function to get
  ** called after processing all the headers in a response
  ** message.  So, if we've been to asked to authenticate a
  ** particular stage, and we've done it, we're OK.  But, if
  ** we're already completely authenticated, it's not OK to
  ** get another 401 or 407.
  **
  ** It is possible for authentication to go stale such that
  ** the client needs to reauthenticate.  Once that info is
  ** available, use it here.
  */
#if 0 /* set to 1 when debugging this functionality */
  infof(data,"%s: authstage = %d\n",__FUNCTION__,data->state.authstage);
  infof(data,"%s: authwant = 0x%08x\n",__FUNCTION__,data->state.authwant);
  infof(data,"%s: authavail = 0x%08x\n",__FUNCTION__,data->state.authavail);
  infof(data,"%s: httpcode = %d\n",__FUNCTION__,k->httpcode);
  infof(data,"%s: authdone = %d\n",__FUNCTION__,data->state.authdone);
  infof(data,"%s: newurl = %s\n",__FUNCTION__,data->req.newurl ?
        data->req.newurl : "(null)");
  infof(data,"%s: authproblem = %d\n",__FUNCTION__,data->state.authproblem);
#endif

  /*
  ** Either we're not authenticating, or we're supposed to
  ** be authenticating something else.  This is an error.
  */
  if((httpcode == 401) && !conn->bits.user_passwd)
    return TRUE;
  if((httpcode == 407) && !conn->bits.proxy_user_passwd)
    return TRUE;

  return data->state.authproblem;
}