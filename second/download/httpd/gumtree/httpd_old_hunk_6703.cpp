#else
    q.dsize = strlen(q.dptr) + 1;
#endif


    if (!(f = dbm_open(auth_dbmpwfile, O_RDONLY, 0664))) {
	ap_log_error(APLOG_MARK, APLOG_ERR, r->server,
		    "could not open dbm auth file: %s", auth_dbmpwfile);
	return NULL;
    }

    d = dbm_fetch(f, q);

