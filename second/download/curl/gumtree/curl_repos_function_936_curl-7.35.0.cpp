static bool imap_endofresp(struct connectdata *conn, char *line, size_t len,
                           int *resp)
{
  struct IMAP *imap = conn->data->req.protop;
  struct imap_conn *imapc = &conn->proto.imapc;
  const char *id = imapc->resptag;
  size_t id_len = strlen(id);

  /* Do we have a tagged command response? */
  if(len >= id_len + 1 && !memcmp(id, line, id_len) && line[id_len] == ' ') {
    line += id_len + 1;
    len -= id_len + 1;

    if(len >= 2 && !memcmp(line, "OK", 2))
      *resp = 'O';
    else if(len >= 2 && !memcmp(line, "NO", 2))
      *resp = 'N';
    else if(len >= 3 && !memcmp(line, "BAD", 3))
      *resp = 'B';
    else {
      failf(conn->data, "Bad tagged response");
      *resp = -1;
    }

    return TRUE;
  }

  /* Do we have an untagged command response? */
  if(len >= 2 && !memcmp("* ", line, 2)) {
    switch(imapc->state) {
      /* States which are interested in untagged responses */
      case IMAP_CAPABILITY:
        if(!imap_matchresp(line, len, "CAPABILITY"))
          return FALSE;
        break;

      case IMAP_LIST:
        if((!imap->custom && !imap_matchresp(line, len, "LIST")) ||
          (imap->custom && !imap_matchresp(line, len, imap->custom) &&
           (strcmp(imap->custom, "STORE") ||
            !imap_matchresp(line, len, "FETCH")) &&
           strcmp(imap->custom, "SELECT") &&
           strcmp(imap->custom, "EXAMINE") &&
           strcmp(imap->custom, "SEARCH") &&
           strcmp(imap->custom, "EXPUNGE") &&
           strcmp(imap->custom, "LSUB") &&
           strcmp(imap->custom, "UID") &&
           strcmp(imap->custom, "NOOP")))
          return FALSE;
        break;

      case IMAP_SELECT:
        /* SELECT is special in that its untagged responses do not have a
           common prefix so accept anything! */
        break;

      case IMAP_FETCH:
        if(!imap_matchresp(line, len, "FETCH"))
          return FALSE;
        break;

      /* Ignore other untagged responses */
      default:
        return FALSE;
    }

    *resp = '*';
    return TRUE;
  }

  /* Do we have a continuation response? This should be a + symbol followed by
     a space and optionally some text as per RFC-3501 for the AUTHENTICATE and
     APPEND commands and as outlined in Section 4. Examples of RFC-4959 but
     some e-mail servers ignore this and only send a single + instead. */
  if((len == 3 && !memcmp("+", line, 1)) ||
     (len >= 2 && !memcmp("+ ", line, 2))) {
    switch(imapc->state) {
      /* States which are interested in continuation responses */
      case IMAP_AUTHENTICATE_PLAIN:
      case IMAP_AUTHENTICATE_LOGIN:
      case IMAP_AUTHENTICATE_LOGIN_PASSWD:
      case IMAP_AUTHENTICATE_CRAMMD5:
      case IMAP_AUTHENTICATE_DIGESTMD5:
      case IMAP_AUTHENTICATE_DIGESTMD5_RESP:
      case IMAP_AUTHENTICATE_NTLM:
      case IMAP_AUTHENTICATE_NTLM_TYPE2MSG:
      case IMAP_AUTHENTICATE_XOAUTH2:
      case IMAP_AUTHENTICATE_FINAL:
      case IMAP_APPEND:
        *resp = '+';
        break;

      default:
        failf(conn->data, "Unexpected continuation response");
        *resp = -1;
        break;
    }

    return TRUE;
  }

  return FALSE; /* Nothing for us */
}