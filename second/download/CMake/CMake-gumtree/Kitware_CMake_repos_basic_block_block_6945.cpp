{
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  struct Curl_easy *data = conn->data;
  const bool blocking = (done == NULL);
  CURLcode result;

  if(connssl->state == ssl_connection_complete) {
    if(!blocking)
      *done = TRUE;
    return CURLE_OK;
  }

  if(connssl->connecting_state == ssl_connect_1) {
    result = nss_setup_connect(conn, sockindex);
    if(result)
      /* we do not expect CURLE_AGAIN from nss_setup_connect() */
      return result;

    connssl->connecting_state = ssl_connect_2;
  }

  /* enable/disable blocking mode before handshake */
  result = nss_set_blocking(connssl, data, blocking);
  if(result)
    return result;

  result = nss_do_connect(conn, sockindex);
  switch(result) {
  case CURLE_OK:
    break;
  case CURLE_AGAIN:
    if(!blocking)
      /* CURLE_AGAIN in non-blocking mode is not an error */
      return CURLE_OK;
    /* fall through */
  default:
    return result;
  }

  if(blocking) {
    /* in blocking mode, set NSS non-blocking mode _after_ SSL handshake */
    result = nss_set_blocking(connssl, data, /* blocking */ FALSE);
    if(result)
      return result;
  }
  else
    /* signal completed SSL handshake */
    *done = TRUE;

  connssl->state = ssl_connection_complete;
  conn->recv[sockindex] = nss_recv;
  conn->send[sockindex] = nss_send;

  /* ssl_connect_done is never used outside, go back to the initial state */
  connssl->connecting_state = ssl_connect_1;

  return CURLE_OK;
}