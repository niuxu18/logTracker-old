		strbuf_release(&err);
		goto rollbacklog;
	}

	flag = log_all_ref_updates;
	log_all_ref_updates = LOG_REFS_NONE;
	if (write_ref_to_lockfile(lock, &orig_oid, &err) ||
	    commit_ref_update(refs, lock, &orig_oid, NULL, &err)) {
		error("unable to write current sha1 into %s: %s", oldrefname, err.buf);
		strbuf_release(&err);
	}
	log_all_ref_updates = flag;

 rollbacklog:
