	   && ((!sec->auth_anon_mustemail) || strlen(sent_pw))
    /* does the password look like an email address ? */
	   && ((!sec->auth_anon_verifyemail)
	       || ((strpbrk("@", sent_pw) != NULL)
		   && (strpbrk(".", sent_pw) != NULL)))) {
	if (sec->auth_anon_logemail && ap_is_initial_req(r)) {
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_INFO, r,
			"Anonymous: Passwd <%s> Accepted",
			sent_pw ? sent_pw : "\'none\'");
	}
	return OK;
    }
    else {
	if (sec->auth_anon_authoritative) {
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
			"Anonymous: Authoritative, Passwd <%s> not accepted",
			sent_pw ? sent_pw : "\'none\'");
	    return AUTH_REQUIRED;
	}
	/* Drop out the bottom to return DECLINED */
    }
++ apache_1.3.2/src/modules/standard/mod_auth.c	1998-08-07 01:30:54.000000000 +0800
