static int get_request(curl_socket_t sock, struct httprequest *req)
{
  int error;
  int fail = 0;
  char *reqbuf = req->reqbuf;
  ssize_t got = 0;
  int overflow = 0;

  char *pipereq = NULL;
  size_t pipereq_length = 0;

  if(req->pipelining) {
    pipereq = reqbuf + req->checkindex;
    pipereq_length = req->offset - req->checkindex;

    /* Now that we've got the pipelining info we can reset the
       pipelining-related vars which were skipped in init_httprequest */
    req->pipelining = FALSE;
    req->checkindex = 0;
    req->offset = 0;
  }

  if(req->offset >= REQBUFSIZ-1) {
    /* buffer is already full; do nothing */
    overflow = 1;
  }
  else {
    if(pipereq_length && pipereq) {
      memmove(reqbuf, pipereq, pipereq_length);
      got = curlx_uztosz(pipereq_length);
      pipereq_length = 0;
    }
    else {
      if(req->skip)
        /* we are instructed to not read the entire thing, so we make sure to
           only read what we're supposed to and NOT read the enire thing the
           client wants to send! */
        got = sread(sock, reqbuf + req->offset, req->cl);
      else
        got = sread(sock, reqbuf + req->offset, REQBUFSIZ-1 - req->offset);
    }
    if(got_exit_signal)
      return -1;
    if(got == 0) {
      logmsg("Connection closed by client");
      fail = 1;
    }
    else if(got < 0) {
      error = SOCKERRNO;
      if(EAGAIN == error || EWOULDBLOCK == error) {
        /* nothing to read at the moment */
        return 0;
      }
      logmsg("recv() returned error: (%d) %s", error, strerror(error));
      fail = 1;
    }
    if(fail) {
      /* dump the request received so far to the external file */
      reqbuf[req->offset] = '\0';
      storerequest(reqbuf, req->offset);
      return -1;
    }

    logmsg("Read %zd bytes", got);

    req->offset += (size_t)got;
    reqbuf[req->offset] = '\0';

    req->done_processing = ProcessRequest(req);
    if(got_exit_signal)
      return -1;
    if(req->done_processing && req->pipe) {
      logmsg("Waiting for another piped request");
      req->done_processing = 0;
      req->pipe--;
    }
  }

  if(overflow || (req->offset == REQBUFSIZ-1 && got > 0)) {
    logmsg("Request would overflow buffer, closing connection");
    /* dump request received so far to external file anyway */
    reqbuf[REQBUFSIZ-1] = '\0';
    fail = 1;
  }
  else if(req->offset > REQBUFSIZ-1) {
    logmsg("Request buffer overflow, closing connection");
    /* dump request received so far to external file anyway */
    reqbuf[REQBUFSIZ-1] = '\0';
    fail = 1;
  }
  else
    reqbuf[req->offset] = '\0';

  /* at the end of a request dump it to an external file */
  if(fail || req->done_processing)
    storerequest(reqbuf, req->pipelining ? req->checkindex : req->offset);
  if(got_exit_signal)
    return -1;

  return fail ? -1 : 1;
}