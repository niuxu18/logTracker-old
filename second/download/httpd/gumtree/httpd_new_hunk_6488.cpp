	    return OK;
	}

	/* if we see a bogus header don't ignore it. Shout and scream */

	if (!(l = strchr(w, ':'))) {
	    ap_log_rerror(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, r,
			"malformed header in meta file: %s", r->filename);
	    return SERVER_ERROR;
	}

	*l++ = '\0';
	while (*l && ap_isspace(*l))
