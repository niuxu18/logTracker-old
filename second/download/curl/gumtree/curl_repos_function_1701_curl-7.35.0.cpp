int Curl_sec_read_msg(struct connectdata *conn, char *buffer,
                      enum protection_level level)
{
  /* decoded_len should be size_t or ssize_t but conn->mech->decode returns an
     int */
  int decoded_len;
  char *buf;
  int ret_code;
  size_t decoded_sz = 0;
  CURLcode error;

  DEBUGASSERT(level > PROT_NONE && level < PROT_LAST);

  error = Curl_base64_decode(buffer + 4, (unsigned char **)&buf, &decoded_sz);
  if(error || decoded_sz == 0)
    return -1;

  if(decoded_sz > (size_t)INT_MAX) {
    free(buf);
    return -1;
  }
  decoded_len = curlx_uztosi(decoded_sz);

  decoded_len = conn->mech->decode(conn->app_data, buf, decoded_len,
                                   level, conn);
  if(decoded_len <= 0) {
    free(buf);
    return -1;
  }

  if(conn->data->set.verbose) {
    buf[decoded_len] = '\n';
    Curl_debug(conn->data, CURLINFO_HEADER_IN, buf, decoded_len + 1, conn);
  }

  buf[decoded_len] = '\0';
  DEBUGASSERT(decoded_len > 3);
  if(buf[3] == '-')
    ret_code = 0;
  else {
    /* Check for error? */
    sscanf(buf, "%d", &ret_code);
  }

  if(buf[decoded_len - 1] == '\n')
    buf[decoded_len - 1] = '\0';
  /* FIXME: Is |buffer| length always greater than |decoded_len|? */
  strcpy(buffer, buf);
  free(buf);
  return ret_code;
}