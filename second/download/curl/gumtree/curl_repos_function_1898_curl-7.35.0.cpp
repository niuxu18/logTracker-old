static CURLcode smtp_calc_sasl_details(struct connectdata *conn,
                                       const char **mech,
                                       char **initresp, size_t *len,
                                       smtpstate *state1, smtpstate *state2)
{
  CURLcode result = CURLE_OK;
  struct SessionHandle *data = conn->data;
  struct smtp_conn *smtpc = &conn->proto.smtpc;

  /* Calculate the supported authentication mechanism, by decreasing order of
     security, as well as the initial response where appropriate */
#ifndef CURL_DISABLE_CRYPTO_AUTH
  if((smtpc->authmechs & SASL_MECH_DIGEST_MD5) &&
     (smtpc->prefmech & SASL_MECH_DIGEST_MD5)) {
    *mech = SASL_MECH_STRING_DIGEST_MD5;
    *state1 = SMTP_AUTH_DIGESTMD5;
    smtpc->authused = SASL_MECH_DIGEST_MD5;
  }
  else if((smtpc->authmechs & SASL_MECH_CRAM_MD5) &&
          (smtpc->prefmech & SASL_MECH_CRAM_MD5)) {
    *mech = SASL_MECH_STRING_CRAM_MD5;
    *state1 = SMTP_AUTH_CRAMMD5;
    smtpc->authused = SASL_MECH_CRAM_MD5;
  }
  else
#endif
#ifdef USE_NTLM
  if((smtpc->authmechs & SASL_MECH_NTLM) &&
     (smtpc->prefmech & SASL_MECH_NTLM)) {
    *mech = SASL_MECH_STRING_NTLM;
    *state1 = SMTP_AUTH_NTLM;
    *state2 = SMTP_AUTH_NTLM_TYPE2MSG;
    smtpc->authused = SASL_MECH_NTLM;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_ntlm_type1_message(conn->user, conn->passwd,
                                                   &conn->ntlm,
                                                   initresp, len);
    }
  else
#endif
  if(((smtpc->authmechs & SASL_MECH_XOAUTH2) &&
      (smtpc->prefmech & SASL_MECH_XOAUTH2) &&
      (smtpc->prefmech != SASL_AUTH_ANY)) || conn->xoauth2_bearer) {
    *mech = SASL_MECH_STRING_XOAUTH2;
    *state1 = SMTP_AUTH_XOAUTH2;
    *state2 = SMTP_AUTH_FINAL;
    smtpc->authused = SASL_MECH_XOAUTH2;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_xoauth2_message(data, conn->user,
                                                conn->xoauth2_bearer,
                                                initresp, len);
  }
  else if((smtpc->authmechs & SASL_MECH_LOGIN) &&
          (smtpc->prefmech & SASL_MECH_LOGIN)) {
    *mech = SASL_MECH_STRING_LOGIN;
    *state1 = SMTP_AUTH_LOGIN;
    *state2 = SMTP_AUTH_LOGIN_PASSWD;
    smtpc->authused = SASL_MECH_LOGIN;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_login_message(data, conn->user, initresp, len);
  }
  else if((smtpc->authmechs & SASL_MECH_PLAIN) &&
          (smtpc->prefmech & SASL_MECH_PLAIN)) {
    *mech = SASL_MECH_STRING_PLAIN;
    *state1 = SMTP_AUTH_PLAIN;
    *state2 = SMTP_AUTH_FINAL;
    smtpc->authused = SASL_MECH_PLAIN;

    if(data->set.sasl_ir)
      result = Curl_sasl_create_plain_message(data, conn->user, conn->passwd,
                                              initresp, len);
  }

  return result;
}