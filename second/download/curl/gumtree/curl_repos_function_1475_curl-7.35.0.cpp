static CURLcode pop3_calc_sasl_details(struct connectdata *conn,
                                       const char **mech,
                                       char **initresp, size_t *len,
                                       pop3state *state1, pop3state *state2)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  struct pop3_conn *pop3c = &conn->proto.pop3c;

  /* Calculate the supported authentication mechanism, by decreasing order of
     security, as well as the initial response where appropriate */
#ifndef CURL_DISABLE_CRYPTO_AUTH
  if((pop3c->authmechs & SASL_MECH_DIGEST_MD5) &&
      (pop3c->prefmech & SASL_MECH_DIGEST_MD5)) {
    *mech = SASL_MECH_STRING_DIGEST_MD5;
    *state1 = POP3_AUTH_DIGESTMD5;
    pop3c->authused = SASL_MECH_DIGEST_MD5;
  }
  else if((pop3c->authmechs & SASL_MECH_CRAM_MD5) &&
          (pop3c->prefmech & SASL_MECH_CRAM_MD5)) {
    *mech = SASL_MECH_STRING_CRAM_MD5;
    *state1 = POP3_AUTH_CRAMMD5;
    pop3c->authused = SASL_MECH_CRAM_MD5;
  }
  else
#endif
#ifdef USE_NTLM
  if((pop3c->authmechs & SASL_MECH_NTLM) &&
      (pop3c->prefmech & SASL_MECH_NTLM)) {
    *mech = SASL_MECH_STRING_NTLM;
    *state1 = POP3_AUTH_NTLM;
    *state2 = POP3_AUTH_NTLM_TYPE2MSG;
    pop3c->authused = SASL_MECH_NTLM;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_ntlm_type1_message(conn->user, conn->passwd,
                                                    &conn->ntlm,
                                                    initresp, len);
  }
  else
#endif
  if(((pop3c->authmechs & SASL_MECH_XOAUTH2) &&
      (pop3c->prefmech & SASL_MECH_XOAUTH2) &&
      (pop3c->prefmech != SASL_AUTH_ANY)) || conn->xoauth2_bearer) {
    *mech = SASL_MECH_STRING_XOAUTH2;
    *state1 = POP3_AUTH_XOAUTH2;
    *state2 = POP3_AUTH_FINAL;
    pop3c->authused = SASL_MECH_XOAUTH2;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_xoauth2_message(data, conn->user,
                                                conn->xoauth2_bearer,
                                                initresp, len);
  }
  else if((pop3c->authmechs & SASL_MECH_LOGIN) &&
          (pop3c->prefmech & SASL_MECH_LOGIN)) {
    *mech = SASL_MECH_STRING_LOGIN;
    *state1 = POP3_AUTH_LOGIN;
    *state2 = POP3_AUTH_LOGIN_PASSWD;
    pop3c->authused = SASL_MECH_LOGIN;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_login_message(data, conn->user, initresp, len);
  }
  else if((pop3c->authmechs & SASL_MECH_PLAIN) &&
          (pop3c->prefmech & SASL_MECH_PLAIN)) {
    *mech = SASL_MECH_STRING_PLAIN;
    *state1 = POP3_AUTH_PLAIN;
    *state2 = POP3_AUTH_FINAL;
    pop3c->authused = SASL_MECH_PLAIN;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_plain_message(data, conn->user, conn->passwd,
                                              initresp, len);
  }

  return result;
}