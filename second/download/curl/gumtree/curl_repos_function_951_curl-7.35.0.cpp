static CURLcode imap_state_capability_resp(struct connectdata *conn,
                                           int imapcode,
                                           imapstate instate)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  struct imap_conn *imapc = &conn->proto.imapc;
  const char *line = data->state.buffer;
  size_t wordlen;

  (void)instate; /* no use for this yet */

  /* Do we have a untagged response? */
  if(imapcode == '*') {
    line += 2;

    /* Loop through the data line */
    for(;;) {
      while(*line &&
            (*line == ' ' || *line == '\t' ||
              *line == '\r' || *line == '\n')) {

        line++;
      }

      if(!*line)
        break;

      /* Extract the word */
      for(wordlen = 0; line[wordlen] && line[wordlen] != ' ' &&
            line[wordlen] != '\t' && line[wordlen] != '\r' &&
            line[wordlen] != '\n';)
        wordlen++;

      /* Does the server support the STARTTLS capability? */
      if(wordlen == 8 && !memcmp(line, "STARTTLS", 8))
        imapc->tls_supported = TRUE;

      /* Has the server explicitly disabled clear text authentication? */
      else if(wordlen == 13 && !memcmp(line, "LOGINDISABLED", 13))
        imapc->login_disabled = TRUE;

      /* Does the server support the SASL-IR capability? */
      else if(wordlen == 7 && !memcmp(line, "SASL-IR", 7))
        imapc->ir_supported = TRUE;

      /* Do we have a SASL based authentication mechanism? */
      else if(wordlen > 5 && !memcmp(line, "AUTH=", 5)) {
        line += 5;
        wordlen -= 5;

        /* Test the word for a matching authentication mechanism */
        if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_LOGIN))
          imapc->authmechs |= SASL_MECH_LOGIN;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_PLAIN))
          imapc->authmechs |= SASL_MECH_PLAIN;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_CRAM_MD5))
          imapc->authmechs |= SASL_MECH_CRAM_MD5;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_DIGEST_MD5))
          imapc->authmechs |= SASL_MECH_DIGEST_MD5;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_GSSAPI))
          imapc->authmechs |= SASL_MECH_GSSAPI;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_EXTERNAL))
          imapc->authmechs |= SASL_MECH_EXTERNAL;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_NTLM))
          imapc->authmechs |= SASL_MECH_NTLM;
        else if(sasl_mech_equal(line, wordlen, SASL_MECH_STRING_XOAUTH2))
          imapc->authmechs |= SASL_MECH_XOAUTH2;
      }

      line += wordlen;
    }
  }
  else if(imapcode == 'O') {
    if(data->set.use_ssl && !conn->ssl[FIRSTSOCKET].use) {
      /* We don't have a SSL/TLS connection yet, but SSL is requested */
      if(imapc->tls_supported)
        /* Switch to TLS connection now */
        result = imap_perform_starttls(conn);
      else if(data->set.use_ssl == CURLUSESSL_TRY)
        /* Fallback and carry on with authentication */
        result = imap_perform_authentication(conn);
      else {
        failf(data, "STARTTLS not supported.");
        result = CURLE_USE_SSL_FAILED;
      }
    }
    else
      result = imap_perform_authentication(conn);
  }
  else
    result = imap_perform_authentication(conn);

  return result;
}