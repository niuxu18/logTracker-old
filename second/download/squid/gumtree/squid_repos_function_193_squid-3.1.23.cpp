char *
ntlm_check_auth(ntlm_authenticate * auth, int auth_length)
{
    int rv;
    char pass[MAX_PASSWD_LEN+1];
    char *domain = credentials;
    char *user;
    lstring tmp;

    if (handle == NULL) {	/*if null we aren't connected, but it shouldn't happen */
        debug("Weird, we've been disconnected\n");
        ntlm_errno = NTLM_NOT_CONNECTED;
        return NULL;
    }

    /*      debug("fetching domain\n"); */
    tmp = ntlm_fetch_string((char *) auth, auth_length, &auth->domain);
    if (tmp.str == NULL || tmp.l == 0) {
        debug("No domain supplied. Returning no-auth\n");
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }
    if (tmp.l > MAX_DOMAIN_LEN) {
        debug("Domain string exceeds %d bytes, rejecting\n", MAX_DOMAIN_LEN);
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }
    memcpy(domain, tmp.str, tmp.l);
    user = domain + tmp.l;
    *user++ = '\0';

    /*      debug("fetching user name\n"); */
    tmp = ntlm_fetch_string((char *) auth, auth_length, &auth->user);
    if (tmp.str == NULL || tmp.l == 0) {
        debug("No username supplied. Returning no-auth\n");
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }
    if (tmp.l > MAX_USERNAME_LEN) {
        debug("Username string exceeds %d bytes, rejecting\n", MAX_USERNAME_LEN);
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }
    memcpy(user, tmp.str, tmp.l);
    *(user + tmp.l) = '\0';


    /* Authenticating against the NT response doesn't seem to work... */
    tmp = ntlm_fetch_string((char *) auth, auth_length, &auth->lmresponse);
    if (tmp.str == NULL || tmp.l == 0) {
        fprintf(stderr, "No auth at all. Returning no-auth\n");
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }
    if (tmp.l > MAX_PASSWD_LEN) {
        debug("Password string exceeds %d bytes, rejecting\n", MAX_PASSWD_LEN);
        ntlm_errno = NTLM_LOGON_ERROR;
        return NULL;
    }

    memcpy(pass, tmp.str, tmp.l);
    pass[min(MAX_PASSWD_LEN,tmp.l)] = '\0';

#if 1
    debug ("Empty LM pass detection: user: '%s', ours:'%s', his: '%s'"
           "(length: %d)\n",
           user,lmencoded_empty_pass,tmp.str,tmp.l);
    if (memcmp(tmp.str,lmencoded_empty_pass,ENCODED_PASS_LEN)==0) {
        fprintf(stderr,"Empty LM password supplied for user %s\\%s. "
                "No-auth\n",domain,user);
        ntlm_errno=NTLM_LOGON_ERROR;
        return NULL;
    }

    tmp = ntlm_fetch_string ((char *) auth, auth_length, &auth->ntresponse);
    if (tmp.str != NULL && tmp.l != 0) {
        debug ("Empty NT pass detection: user: '%s', ours:'%s', his: '%s'"
               "(length: %d)\n",
               user,ntencoded_empty_pass,tmp.str,tmp.l);
        if (memcmp(tmp.str,lmencoded_empty_pass,ENCODED_PASS_LEN)==0) {
            fprintf(stderr,"Empty NT password supplied for user %s\\%s. "
                    "No-auth\n",domain,user);
            ntlm_errno=NTLM_LOGON_ERROR;
            return NULL;
        }
    }
#endif

    /* TODO: check against empty password!!!!! */



    debug("checking domain: '%s', user: '%s', pass='%s'\n", domain, user, pass);

    rv = SMB_Logon_Server(handle, user, pass, domain, 1);
    debug("Login attempt had result %d\n", rv);

    if (rv != NTV_NO_ERROR) {	/* failed */
        ntlm_errno = rv;
        return NULL;
    }
    *(user - 1) = '\\';		/* hack. Performing, but ugly. */

    debug("credentials: %s\n", credentials);
    return credentials;
}