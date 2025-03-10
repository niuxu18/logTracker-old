static void suboption(struct connectdata *conn)
{
  struct curl_slist *v;
  unsigned char temp[2048];
  ssize_t bytes_written;
  size_t len;
  size_t tmplen;
  int err;
  char varname[128];
  char varval[128];
  struct SessionHandle *data = conn->data;
  struct TELNET *tn = (struct TELNET *)data->req.protop;

  printsub(data, '<', (unsigned char *)tn->subbuffer, CURL_SB_LEN(tn)+2);
  switch (CURL_SB_GET(tn)) {
    case CURL_TELOPT_TTYPE:
      len = strlen(tn->subopt_ttype) + 4 + 2;
      snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c%s%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_TTYPE,
               CURL_TELQUAL_IS, tn->subopt_ttype, CURL_IAC, CURL_SE);
      bytes_written = swrite(conn->sock[FIRSTSOCKET], temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        failf(data,"Sending data failed (%d)",err);
      }
      printsub(data, '>', &temp[2], len-2);
      break;
    case CURL_TELOPT_XDISPLOC:
      len = strlen(tn->subopt_xdisploc) + 4 + 2;
      snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c%s%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_XDISPLOC,
               CURL_TELQUAL_IS, tn->subopt_xdisploc, CURL_IAC, CURL_SE);
      bytes_written = swrite(conn->sock[FIRSTSOCKET], temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        failf(data,"Sending data failed (%d)",err);
      }
      printsub(data, '>', &temp[2], len-2);
      break;
    case CURL_TELOPT_NEW_ENVIRON:
      snprintf((char *)temp, sizeof(temp),
               "%c%c%c%c", CURL_IAC, CURL_SB, CURL_TELOPT_NEW_ENVIRON,
               CURL_TELQUAL_IS);
      len = 4;

      for(v = tn->telnet_vars;v;v = v->next) {
        tmplen = (strlen(v->data) + 1);
        /* Add the variable only if it fits */
        if(len + tmplen < (int)sizeof(temp)-6) {
          sscanf(v->data, "%127[^,],%127s", varname, varval);
          snprintf((char *)&temp[len], sizeof(temp) - len,
                   "%c%s%c%s", CURL_NEW_ENV_VAR, varname,
                   CURL_NEW_ENV_VALUE, varval);
          len += tmplen;
        }
      }
      snprintf((char *)&temp[len], sizeof(temp) - len,
               "%c%c", CURL_IAC, CURL_SE);
      len += 2;
      bytes_written = swrite(conn->sock[FIRSTSOCKET], temp, len);
      if(bytes_written < 0) {
        err = SOCKERRNO;
        failf(data,"Sending data failed (%d)",err);
      }
      printsub(data, '>', &temp[2], len-2);
      break;
  }
  return;
}