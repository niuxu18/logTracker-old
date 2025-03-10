CURLcode Curl_ftpsend(struct connectdata *conn, const char *cmd)
{
  ssize_t bytes_written;
#define SBUF_SIZE 1024
  char s[SBUF_SIZE];
  size_t write_len;
  char *sptr=s;
  CURLcode result = CURLE_OK;
#ifdef HAVE_GSSAPI
  enum protection_level data_sec = conn->data_prot;
#endif

  write_len = strlen(cmd);
  if(write_len > (sizeof(s) -3))
    return CURLE_BAD_FUNCTION_ARGUMENT;

  strcpy(&s[write_len], "\r\n"); /* append a trailing CRLF */
  write_len +=2;

  bytes_written=0;

  result = Curl_convert_to_network(conn->data, s, write_len);
  /* Curl_convert_to_network calls failf if unsuccessful */
  if(result)
    return result;

  for(;;) {
#ifdef HAVE_GSSAPI
    conn->data_prot = PROT_CMD;
#endif
    result = Curl_write(conn, conn->sock[FIRSTSOCKET], sptr, write_len,
                        &bytes_written);
#ifdef HAVE_GSSAPI
    DEBUGASSERT(data_sec > PROT_NONE && data_sec < PROT_LAST);
    conn->data_prot = data_sec;
#endif

    if(result)
      break;

    if(conn->data->set.verbose)
      Curl_debug(conn->data, CURLINFO_HEADER_OUT,
                 sptr, (size_t)bytes_written, conn);

    if(bytes_written != (ssize_t)write_len) {
      write_len -= bytes_written;
      sptr += bytes_written;
    }
    else
      break;
  }

  return result;
}