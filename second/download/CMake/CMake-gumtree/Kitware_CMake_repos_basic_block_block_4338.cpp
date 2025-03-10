{
  int subversion=0;
  struct Curl_easy *data=conn->data;
  struct SingleRequest *k = &data->req;
  CURLcode result;
  curl_socket_t tunnelsocket = conn->sock[sockindex];
  curl_off_t cl=0;
  bool closeConnection = FALSE;
  bool chunked_encoding = FALSE;
  time_t check;

#define SELECT_OK      0
#define SELECT_ERROR   1
#define SELECT_TIMEOUT 2
  int error = SELECT_OK;

  if(conn->tunnel_state[sockindex] == TUNNEL_COMPLETE)
    return CURLE_OK; /* CONNECT is already completed */

  conn->bits.proxy_connect_closed = FALSE;

  do {
    if(TUNNEL_INIT == conn->tunnel_state[sockindex]) {
      /* BEGIN CONNECT PHASE */
      char *host_port;
      Curl_send_buffer *req_buffer;

      infof(data, "Establish HTTP proxy tunnel to %s:%hu\n",
            hostname, remote_port);

        /* This only happens if we've looped here due to authentication
           reasons, and we don't really use the newly cloned URL here
           then. Just free() it. */
      free(data->req.newurl);
      data->req.newurl = NULL;

      /* initialize a dynamic send-buffer */
      req_buffer = Curl_add_buffer_init();

      if(!req_buffer)
        return CURLE_OUT_OF_MEMORY;

      host_port = aprintf("%s:%hu", hostname, remote_port);
      if(!host_port) {
        Curl_add_buffer_free(req_buffer);
        return CURLE_OUT_OF_MEMORY;
      }

      /* Setup the proxy-authorization header, if any */
      result = Curl_http_output_auth(conn, "CONNECT", host_port, TRUE);

      free(host_port);

      if(!result) {
        char *host = NULL;
        const char *proxyconn="";
        const char *useragent="";
        const char *http = (conn->http_proxy.proxytype == CURLPROXY_HTTP_1_0) ?
          "1.0" : "1.1";
        bool ipv6_ip = conn->bits.ipv6_ip;
        char *hostheader;

        /* the hostname may be different */
        if(hostname != conn->host.name)
          ipv6_ip = (strchr(hostname, ':') != NULL);
        hostheader= /* host:port with IPv6 support */
          aprintf("%s%s%s:%hu", ipv6_ip?"[":"", hostname, ipv6_ip?"]":"",
                  remote_port);
        if(!hostheader) {
          Curl_add_buffer_free(req_buffer);
          return CURLE_OUT_OF_MEMORY;
        }

        if(!Curl_checkProxyheaders(conn, "Host:")) {
          host = aprintf("Host: %s\r\n", hostheader);
          if(!host) {
            free(hostheader);
            Curl_add_buffer_free(req_buffer);
            return CURLE_OUT_OF_MEMORY;
          }
        }
        if(!Curl_checkProxyheaders(conn, "Proxy-Connection:"))
          proxyconn = "Proxy-Connection: Keep-Alive\r\n";

        if(!Curl_checkProxyheaders(conn, "User-Agent:") &&
           data->set.str[STRING_USERAGENT])
          useragent = conn->allocptr.uagent;

        result =
          Curl_add_bufferf(req_buffer,
                           "CONNECT %s HTTP/%s\r\n"
                           "%s"  /* Host: */
                           "%s"  /* Proxy-Authorization */
                           "%s"  /* User-Agent */
                           "%s", /* Proxy-Connection */
                           hostheader,
                           http,
                           host?host:"",
                           conn->allocptr.proxyuserpwd?
                           conn->allocptr.proxyuserpwd:"",
                           useragent,
                           proxyconn);

        if(host)
          free(host);
        free(hostheader);

        if(!result)
          result = Curl_add_custom_headers(conn, TRUE, req_buffer);

        if(!result)
          /* CRLF terminate the request */
          result = Curl_add_bufferf(req_buffer, "\r\n");

        if(!result) {
          /* Send the connect request to the proxy */
          /* BLOCKING */
          result =
            Curl_add_buffer_send(req_buffer, conn,
                                 &data->info.request_size, 0, sockindex);
        }
        req_buffer = NULL;
        if(result)
          failf(data, "Failed sending CONNECT to proxy");
      }

      Curl_add_buffer_free(req_buffer);
      if(result)
        return result;

      conn->tunnel_state[sockindex] = TUNNEL_CONNECT;
    } /* END CONNECT PHASE */

    check = Curl_timeleft(data, NULL, TRUE);
    if(check <= 0) {
      failf(data, "Proxy CONNECT aborted due to timeout");
      return CURLE_RECV_ERROR;
    }

    if(!Curl_conn_data_pending(conn, sockindex))
      /* return so we'll be called again polling-style */
      return CURLE_OK;
    DEBUGF(infof(data, "Read response immediately from proxy CONNECT\n"));

    /* at this point, the tunnel_connecting phase is over. */

    { /* READING RESPONSE PHASE */
      size_t nread;   /* total size read */
      int perline; /* count bytes per line */
      int keepon=TRUE;
      ssize_t gotbytes;
      char *ptr;
      char *line_start;

      ptr = conn->connect_buffer;
      line_start = ptr;

      nread = 0;
      perline = 0;

      while(nread < (size_t)CONNECT_BUFFER_SIZE && keepon && !error) {
        if(Curl_pgrsUpdate(conn))
          return CURLE_ABORTED_BY_CALLBACK;

        if(ptr >= &conn->connect_buffer[CONNECT_BUFFER_SIZE]) {
          failf(data, "CONNECT response too large!");
          return CURLE_RECV_ERROR;
        }

        check = Curl_timeleft(data, NULL, TRUE);
        if(check <= 0) {
          failf(data, "Proxy CONNECT aborted due to timeout");
          error = SELECT_TIMEOUT; /* already too little time */
          break;
        }

        /* Read one byte at a time to avoid a race condition. Wait at most one
           second before looping to ensure continuous pgrsUpdates. */
        result = Curl_read(conn, tunnelsocket, ptr, 1, &gotbytes);
        if(result == CURLE_AGAIN) {
          if(SOCKET_READABLE(tunnelsocket, check<1000L?check:1000) == -1) {
            error = SELECT_ERROR;
            failf(data, "Proxy CONNECT aborted due to select/poll error");
            break;
          }
          continue;
        }
        if(result) {
          keepon = FALSE;
          break;
        }
        else if(gotbytes <= 0) {
          if(data->set.proxyauth && data->state.authproxy.avail) {
            /* proxy auth was requested and there was proxy auth available,
               then deem this as "mere" proxy disconnect */
            conn->bits.proxy_connect_closed = TRUE;
            infof(data, "Proxy CONNECT connection closed\n");
          }
          else {
            error = SELECT_ERROR;
            failf(data, "Proxy CONNECT aborted");
          }
          keepon = FALSE;
          break;
        }

        /* We got a byte of data */
        nread++;

        if(keepon > TRUE) {
          /* This means we are currently ignoring a response-body */

          nread = 0; /* make next read start over in the read buffer */
          ptr = conn->connect_buffer;
          if(cl) {
            /* A Content-Length based body: simply count down the counter
               and make sure to break out of the loop when we're done! */
            cl--;
            if(cl <= 0) {
              keepon = FALSE;
              break;
            }
          }
          else {
            /* chunked-encoded body, so we need to do the chunked dance
               properly to know when the end of the body is reached */
            CHUNKcode r;
            ssize_t tookcareof = 0;

            /* now parse the chunked piece of data so that we can
               properly tell when the stream ends */
            r = Curl_httpchunk_read(conn, ptr, 1, &tookcareof);
            if(r == CHUNKE_STOP) {
              /* we're done reading chunks! */
              infof(data, "chunk reading DONE\n");
              keepon = FALSE;
              /* we did the full CONNECT treatment, go COMPLETE */
              conn->tunnel_state[sockindex] = TUNNEL_COMPLETE;
            }
          }
          continue;
        }

        perline++; /* amount of bytes in this line so far */

        /* if this is not the end of a header line then continue */
        if(*ptr != 0x0a) {
          ptr++;
          continue;
        }

        /* convert from the network encoding */
        result = Curl_convert_from_network(data, line_start, perline);
        /* Curl_convert_from_network calls failf if unsuccessful */
        if(result)
          return result;

        /* output debug if that is requested */
        if(data->set.verbose)
          Curl_debug(data, CURLINFO_HEADER_IN,
                     line_start, (size_t)perline, conn);

        if(!data->set.suppress_connect_headers) {
          /* send the header to the callback */
          int writetype = CLIENTWRITE_HEADER;
          if(data->set.include_header)
            writetype |= CLIENTWRITE_BODY;

          result = Curl_client_write(conn, writetype, line_start, perline);
          if(result)
            return result;
        }

        data->info.header_size += (long)perline;
        data->req.headerbytecount += (long)perline;

        /* Newlines are CRLF, so the CR is ignored as the line isn't
           really terminated until the LF comes. Treat a following CR
           as end-of-headers as well.*/

        if(('\r' == line_start[0]) ||
           ('\n' == line_start[0])) {
          /* end of response-headers from the proxy */
          nread = 0; /* make next read start over in the read
                        buffer */
          ptr = conn->connect_buffer;
          if((407 == k->httpcode) && !data->state.authproblem) {
            /* If we get a 407 response code with content length
               when we have no auth problem, we must ignore the
               whole response-body */
            keepon = 2;

            if(cl) {
              infof(data, "Ignore %" CURL_FORMAT_CURL_OFF_T
                    " bytes of response-body\n", cl);
            }
            else if(chunked_encoding) {
              CHUNKcode r;

              infof(data, "Ignore chunked response-body\n");

              /* We set ignorebody true here since the chunked
                 decoder function will acknowledge that. Pay
                 attention so that this is cleared again when this
                 function returns! */
              k->ignorebody = TRUE;

              if(line_start[1] == '\n') {
                /* this can only be a LF if the letter at index 0
                   was a CR */
                line_start++;
              }

              /* now parse the chunked piece of data so that we can
                 properly tell when the stream ends */
              r = Curl_httpchunk_read(conn, line_start + 1, 1, &gotbytes);
              if(r == CHUNKE_STOP) {
                /* we're done reading chunks! */
                infof(data, "chunk reading DONE\n");
                keepon = FALSE;
                /* we did the full CONNECT treatment, go to
                   COMPLETE */
                conn->tunnel_state[sockindex] = TUNNEL_COMPLETE;
              }
            }
            else {
              /* without content-length or chunked encoding, we
                 can't keep the connection alive since the close is
                 the end signal so we bail out at once instead */
              keepon = FALSE;
            }
          }
          else
            keepon = FALSE;
          /* we did the full CONNECT treatment, go to COMPLETE */
          conn->tunnel_state[sockindex] = TUNNEL_COMPLETE;
          continue;
        }

        line_start[perline] = 0; /* zero terminate the buffer */
        if((checkprefix("WWW-Authenticate:", line_start) &&
            (401 == k->httpcode)) ||
           (checkprefix("Proxy-authenticate:", line_start) &&
            (407 == k->httpcode))) {

          bool proxy = (k->httpcode == 407) ? TRUE : FALSE;
          char *auth = Curl_copy_header_value(line_start);
          if(!auth)
            return CURLE_OUT_OF_MEMORY;

          result = Curl_http_input_auth(conn, proxy, auth);

          free(auth);

          if(result)
            return result;
        }
        else if(checkprefix("Content-Length:", line_start)) {
          if(k->httpcode/100 == 2) {
            /* A client MUST ignore any Content-Length or Transfer-Encoding
               header fields received in a successful response to CONNECT.
               "Successful" described as: 2xx (Successful). RFC 7231 4.3.6 */
            infof(data, "Ignoring Content-Length in CONNECT %03d response\n",
                  k->httpcode);
          }
          else {
            cl = curlx_strtoofft(line_start +
                                 strlen("Content-Length:"), NULL, 10);
          }
        }
        else if(Curl_compareheader(line_start, "Connection:", "close"))
          closeConnection = TRUE;
        else if(checkprefix("Transfer-Encoding:", line_start)) {
          if(k->httpcode/100 == 2) {
            /* A client MUST ignore any Content-Length or Transfer-Encoding
               header fields received in a successful response to CONNECT.
               "Successful" described as: 2xx (Successful). RFC 7231 4.3.6 */
            infof(data, "Ignoring Transfer-Encoding in "
                  "CONNECT %03d response\n", k->httpcode);
          }
          else if(Curl_compareheader(line_start,
                                     "Transfer-Encoding:", "chunked")) {
            infof(data, "CONNECT responded chunked\n");
            chunked_encoding = TRUE;
            /* init our chunky engine */
            Curl_httpchunk_init(conn);
          }
        }
        else if(Curl_compareheader(line_start, "Proxy-Connection:", "close"))
          closeConnection = TRUE;
        else if(2 == sscanf(line_start, "HTTP/1.%d %d",
                            &subversion,
                            &k->httpcode)) {
          /* store the HTTP code from the proxy */
          data->info.httpproxycode = k->httpcode;
        }

        perline = 0; /* line starts over here */
        ptr = conn->connect_buffer;
        line_start = ptr;
      } /* while there's buffer left and loop is requested */

      if(Curl_pgrsUpdate(conn))
        return CURLE_ABORTED_BY_CALLBACK;

      if(error)
        return CURLE_RECV_ERROR;

      if(data->info.httpproxycode != 200) {
        /* Deal with the possibly already received authenticate
           headers. 'newurl' is set to a new URL if we must loop. */
        result = Curl_http_auth_act(conn);
        if(result)
          return result;

        if(conn->bits.close)
          /* the connection has been marked for closure, most likely in the
             Curl_http_auth_act() function and thus we can kill it at once
             below */
          closeConnection = TRUE;
      }

      if(closeConnection && data->req.newurl) {
        /* Connection closed by server. Don't use it anymore */
        Curl_closesocket(conn, conn->sock[sockindex]);
        conn->sock[sockindex] = CURL_SOCKET_BAD;
        break;
      }
    } /* END READING RESPONSE PHASE */

    /* If we are supposed to continue and request a new URL, which basically
     * means the HTTP authentication is still going on so if the tunnel
     * is complete we start over in INIT state */
    if(data->req.newurl &&
       (TUNNEL_COMPLETE == conn->tunnel_state[sockindex])) {
      conn->tunnel_state[sockindex] = TUNNEL_INIT;
      infof(data, "TUNNEL_STATE switched to: %d\n",
            conn->tunnel_state[sockindex]);
    }

  } while(data->req.newurl);

  if(200 != data->req.httpcode) {
    if(closeConnection && data->req.newurl) {
      conn->bits.proxy_connect_closed = TRUE;
      infof(data, "Connect me again please\n");
    }
    else {
      free(data->req.newurl);
      data->req.newurl = NULL;
      /* failure, close this connection to avoid re-use */
      streamclose(conn, "proxy CONNECT failure");
      Curl_closesocket(conn, conn->sock[sockindex]);
      conn->sock[sockindex] = CURL_SOCKET_BAD;
    }

    /* to back to init state */
    conn->tunnel_state[sockindex] = TUNNEL_INIT;

    if(conn->bits.proxy_connect_closed)
      /* this is not an error, just part of the connection negotiation */
      return CURLE_OK;
    failf(data, "Received HTTP code %d from proxy after CONNECT",
          data->req.httpcode);
    return CURLE_RECV_ERROR;
  }

  conn->tunnel_state[sockindex] = TUNNEL_COMPLETE;

  /* If a proxy-authorization header was used for the proxy, then we should
     make sure that it isn't accidentally used for the document request
     after we've connected. So let's free and clear it here. */
  Curl_safefree(conn->allocptr.proxyuserpwd);
  conn->allocptr.proxyuserpwd = NULL;

  data->state.authproxy.done = TRUE;

  infof(data, "Proxy replied OK to CONNECT request\n");
  data->req.ignorebody = FALSE; /* put it (back) to non-ignore state */
  conn->bits.rewindaftersend = FALSE; /* make sure this isn't set for the
                                         document request  */
  return CURLE_OK;
}