CURLcode Curl_pp_readresp(curl_socket_t sockfd,
                          struct pingpong *pp,
                          int *code, /* return the server code if done */
                          size_t *size) /* size of the response */
{
  ssize_t perline; /* count bytes per line */
  bool keepon=TRUE;
  ssize_t gotbytes;
  char *ptr;
  struct connectdata *conn = pp->conn;
  struct SessionHandle *data = conn->data;
  char * const buf = data->state.buffer;
  CURLcode result = CURLE_OK;

  *code = 0; /* 0 for errors or not done */
  *size = 0;

  ptr=buf + pp->nread_resp;

  /* number of bytes in the current line, so far */
  perline = (ssize_t)(ptr-pp->linestart_resp);

  keepon=TRUE;

  while((pp->nread_resp<BUFSIZE) && (keepon && !result)) {

    if(pp->cache) {
      /* we had data in the "cache", copy that instead of doing an actual
       * read
       *
       * pp->cache_size is cast to ssize_t here.  This should be safe, because
       * it would have been populated with something of size int to begin
       * with, even though its datatype may be larger than an int.
       */
      DEBUGASSERT((ptr+pp->cache_size) <= (buf+BUFSIZE+1));
      memcpy(ptr, pp->cache, pp->cache_size);
      gotbytes = (ssize_t)pp->cache_size;
      free(pp->cache);    /* free the cache */
      pp->cache = NULL;   /* clear the pointer */
      pp->cache_size = 0; /* zero the size just in case */
    }
    else {
      int res;
#ifdef HAVE_GSSAPI
      enum protection_level prot = conn->data_prot;
      conn->data_prot = PROT_CLEAR;
#endif
      DEBUGASSERT((ptr+BUFSIZE-pp->nread_resp) <= (buf+BUFSIZE+1));
      res = Curl_read(conn, sockfd, ptr, BUFSIZE-pp->nread_resp,
                      &gotbytes);
#ifdef HAVE_GSSAPI
      DEBUGASSERT(prot  > PROT_NONE && prot < PROT_LAST);
      conn->data_prot = prot;
#endif
      if(res == CURLE_AGAIN)
        return CURLE_OK; /* return */

      if((res == CURLE_OK) && (gotbytes > 0))
        /* convert from the network encoding */
        res = Curl_convert_from_network(data, ptr, gotbytes);
      /* Curl_convert_from_network calls failf if unsuccessful */

      if(CURLE_OK != res) {
        result = (CURLcode)res; /* Set outer result variable to this error. */
        keepon = FALSE;
      }
    }

    if(!keepon)
      ;
    else if(gotbytes <= 0) {
      keepon = FALSE;
      result = CURLE_RECV_ERROR;
      failf(data, "response reading failed");
    }
    else {
      /* we got a whole chunk of data, which can be anything from one
       * byte to a set of lines and possible just a piece of the last
       * line */
      ssize_t i;
      ssize_t clipamount = 0;
      bool restart = FALSE;

      data->req.headerbytecount += (long)gotbytes;

      pp->nread_resp += gotbytes;
      for(i = 0; i < gotbytes; ptr++, i++) {
        perline++;
        if(*ptr=='\n') {
          /* a newline is CRLF in pp-talk, so the CR is ignored as
             the line isn't really terminated until the LF comes */

          /* output debug output if that is requested */
#ifdef HAVE_GSSAPI
          if(!conn->sec_complete)
#endif
            if(data->set.verbose)
              Curl_debug(data, CURLINFO_HEADER_IN,
                         pp->linestart_resp, (size_t)perline, conn);

          /*
           * We pass all response-lines to the callback function registered
           * for "headers". The response lines can be seen as a kind of
           * headers.
           */
          result = Curl_client_write(conn, CLIENTWRITE_HEADER,
                                     pp->linestart_resp, perline);
          if(result)
            return result;

          if(pp->endofresp(conn, pp->linestart_resp, perline, code)) {
            /* This is the end of the last line, copy the last line to the
               start of the buffer and zero terminate, for old times sake */
            size_t n = ptr - pp->linestart_resp;
            memmove(buf, pp->linestart_resp, n);
            buf[n]=0; /* zero terminate */
            keepon=FALSE;
            pp->linestart_resp = ptr+1; /* advance pointer */
            i++; /* skip this before getting out */

            *size = pp->nread_resp; /* size of the response */
            pp->nread_resp = 0; /* restart */
            break;
          }
          perline=0; /* line starts over here */
          pp->linestart_resp = ptr+1;
        }
      }

      if(!keepon && (i != gotbytes)) {
        /* We found the end of the response lines, but we didn't parse the
           full chunk of data we have read from the server. We therefore need
           to store the rest of the data to be checked on the next invoke as
           it may actually contain another end of response already! */
        clipamount = gotbytes - i;
        restart = TRUE;
        DEBUGF(infof(data, "Curl_pp_readresp_ %d bytes of trailing "
                     "server response left\n",
                     (int)clipamount));
      }
      else if(keepon) {

        if((perline == gotbytes) && (gotbytes > BUFSIZE/2)) {
          /* We got an excessive line without newlines and we need to deal
             with it. We keep the first bytes of the line then we throw
             away the rest. */
          infof(data, "Excessive server response line length received, "
                "%zd bytes. Stripping\n", gotbytes);
          restart = TRUE;

          /* we keep 40 bytes since all our pingpong protocols are only
             interested in the first piece */
          clipamount = 40;
        }
        else if(pp->nread_resp > BUFSIZE/2) {
          /* We got a large chunk of data and there's potentially still
             trailing data to take care of, so we put any such part in the
             "cache", clear the buffer to make space and restart. */
          clipamount = perline;
          restart = TRUE;
        }
      }
      else if(i == gotbytes)
        restart = TRUE;

      if(clipamount) {
        pp->cache_size = clipamount;
        pp->cache = malloc(pp->cache_size);
        if(pp->cache)
          memcpy(pp->cache, pp->linestart_resp, pp->cache_size);
        else
          return CURLE_OUT_OF_MEMORY;
      }
      if(restart) {
        /* now reset a few variables to start over nicely from the start of
           the big buffer */
        pp->nread_resp = 0; /* start over from scratch in the buffer */
        ptr = pp->linestart_resp = buf;
        perline = 0;
      }

    } /* there was data */

  } /* while there's buffer left and loop is requested */

  pp->pending_resp = FALSE;

  return result;
}