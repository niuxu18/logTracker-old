CURLcode Curl_ConnectHTTPProxyTunnel(struct connectdata *conn,
                                     int sockindex,
                                     char *hostname,
                                     int remote_port)
{
  int subversion=0;
  struct SessionHandle *data=conn->data;
  struct Curl_transfer_keeper *k = &conn->keep;
  CURLcode result;
  int res;
  size_t nread;   /* total size read */
  int perline; /* count bytes per line */
  bool keepon=TRUE;
  ssize_t gotbytes;
  char *ptr;
  long timeout =
    data->set.timeout?data->set.timeout:3600; /* in seconds */
  char *line_start;
  char *host_port;
  curl_socket_t tunnelsocket = conn->sock[sockindex];
  send_buffer *req_buffer;

#define SELECT_OK      0
#define SELECT_ERROR   1
#define SELECT_TIMEOUT 2
  int error = SELECT_OK;

  infof(data, "Establish HTTP proxy tunnel to %s:%d\n", hostname, remote_port);

  do {
    if(conn->newurl) {
      /* This only happens if we've looped here due to authentication reasons,
         and we don't really use the newly cloned URL here then. Just free()
         it. */
      free(conn->newurl);
      conn->newurl = NULL;
    }

    /* initialize a dynamic send-buffer */
    req_buffer = add_buffer_init();

    if(!req_buffer)
      return CURLE_OUT_OF_MEMORY;

    host_port = aprintf("%s:%d", hostname, remote_port);
    if(!host_port)
      return CURLE_OUT_OF_MEMORY;

    /* Setup the proxy-authorization header, if any */
    result = Curl_http_output_auth(conn, (char *)"CONNECT", host_port, TRUE);

    if(CURLE_OK == result) {
      char *host=(char *)"";
      const char *proxyconn="";
      char *ptr;

      ptr = checkheaders(data, "Host:");
      if(!ptr) {
        host = aprintf("Host: %s\r\n", host_port);
        if(!host)
          result = CURLE_OUT_OF_MEMORY;
      }
      ptr = checkheaders(data, "Proxy-Connection:");
      if(!ptr)
        proxyconn = "Proxy-Connection: Keep-Alive\r\n";

      if(CURLE_OK == result) {
        /* Send the connect request to the proxy */
        /* BLOCKING */
        result =
          add_bufferf(req_buffer,
                      "CONNECT %s:%d HTTP/1.0\r\n"
                      "%s"  /* Host: */
                      "%s"  /* Proxy-Authorization */
                      "%s"  /* User-Agent */
                      "%s", /* Proxy-Connection */
                      hostname, remote_port,
                      host,
                      conn->allocptr.proxyuserpwd?
                      conn->allocptr.proxyuserpwd:"",
                      data->set.useragent?conn->allocptr.uagent:"",
                      proxyconn);

        if(CURLE_OK == result)
          result = add_custom_headers(conn, req_buffer);

        if(host && *host)
          free(host);

        if(CURLE_OK == result)
          /* CRLF terminate the request */
          result = add_bufferf(req_buffer, "\r\n");

        if(CURLE_OK == result)
          /* Now send off the request */
          result = add_buffer_send(req_buffer, conn,
                                   &data->info.request_size);
      }
      if(result)
        failf(data, "Failed sending CONNECT to proxy");
    }
    free(host_port);
    if(result)
      return result;

    ptr=data->state.buffer;
    line_start = ptr;

    nread=0;
    perline=0;
    keepon=TRUE;

    while((nread<BUFSIZE) && (keepon && !error)) {

      /* if timeout is requested, find out how much remaining time we have */
      long check = timeout - /* timeout time */
        Curl_tvdiff(Curl_tvnow(), conn->now)/1000; /* spent time */
      if(check <=0 ) {
        failf(data, "Proxy CONNECT aborted due to timeout");
        error = SELECT_TIMEOUT; /* already too little time */
        break;
      }

      /* timeout each second and check the timeout */
      switch (Curl_select(tunnelsocket, CURL_SOCKET_BAD, 1000)) {
      case -1: /* select() error, stop reading */
        error = SELECT_ERROR;
        failf(data, "Proxy CONNECT aborted due to select() error");
        break;
      case 0: /* timeout */
        break;
      default:
        res = Curl_read(conn, tunnelsocket, ptr, BUFSIZE-nread, &gotbytes);
        if(res< 0)
          /* EWOULDBLOCK */
          continue; /* go loop yourself */
        else if(res)
          keepon = FALSE;
        else if(gotbytes <= 0) {
          keepon = FALSE;
          error = SELECT_ERROR;
          failf(data, "Proxy CONNECT aborted");
        }
        else {
          /*
           * We got a whole chunk of data, which can be anything from one byte
           * to a set of lines and possibly just a piece of the last line.
           */
          int i;

          nread += gotbytes;
          for(i = 0; i < gotbytes; ptr++, i++) {
            perline++; /* amount of bytes in this line so far */
            if(*ptr=='\n') {
              char letter;
              int writetype;

              /* output debug if that is requested */
              if(data->set.verbose)
                Curl_debug(data, CURLINFO_HEADER_IN, line_start, perline,
                           conn);

              /* send the header to the callback */
              writetype = CLIENTWRITE_HEADER;
              if(data->set.include_header)
                writetype |= CLIENTWRITE_BODY;

              result = Curl_client_write(data, writetype, line_start, perline);
              if(result)
                return result;

              /* Newlines are CRLF, so the CR is ignored as the line isn't
                 really terminated until the LF comes. Treat a following CR
                 as end-of-headers as well.*/

              if(('\r' == line_start[0]) ||
                 ('\n' == line_start[0])) {
                /* end of response-headers from the proxy */
                keepon=FALSE;
                break; /* breaks out of for-loop, not switch() */
              }

              /* keep a backup of the position we are about to blank */
              letter = line_start[perline];
              line_start[perline]=0; /* zero terminate the buffer */
              if((checkprefix("WWW-Authenticate:", line_start) &&
                  (401 == k->httpcode)) ||
                 (checkprefix("Proxy-authenticate:", line_start) &&
                  (407 == k->httpcode))) {
                result = Curl_http_input_auth(conn, k->httpcode, line_start);
                if(result)
                  return result;
              }
              else if(2 == sscanf(line_start, "HTTP/1.%d %d",
                                  &subversion,
                                  &k->httpcode)) {
                /* store the HTTP code from the proxy */
                data->info.httpproxycode = k->httpcode;
              }
              /* put back the letter we blanked out before */
              line_start[perline]= letter;

              perline=0; /* line starts over here */
              line_start = ptr+1; /* this skips the zero byte we wrote */
            }
          }
        }
        break;
      } /* switch */
    } /* while there's buffer left and loop is requested */

    if(error)
      return CURLE_RECV_ERROR;

    if(data->info.httpproxycode != 200)
      /* Deal with the possibly already received authenticate
         headers. 'newurl' is set to a new URL if we must loop. */
      Curl_http_auth_act(conn);

  } while(conn->newurl);

  if(200 != k->httpcode) {
    failf(data, "Received HTTP code %d from proxy after CONNECT",
          k->httpcode);
    return CURLE_RECV_ERROR;
  }

  /* If a proxy-authorization header was used for the proxy, then we should
     make sure that it isn't accidentally used for the document request
     after we've connected. So let's free and clear it here. */
  Curl_safefree(conn->allocptr.proxyuserpwd);
  conn->allocptr.proxyuserpwd = NULL;

  data->state.authproxy.done = TRUE;

  infof (data, "Proxy replied OK to CONNECT request\n");
  return CURLE_OK;
}