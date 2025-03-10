	}

	if ((commit = lookup_commit_reference(sha1)) == NULL)
		die(_("Not a valid branch point: '%s'."), start_name);
	hashcpy(sha1, commit->object.oid.hash);

	if (reflog)
		log_all_ref_updates = LOG_REFS_NORMAL;

	if (!dont_change_ref) {
		struct ref_transaction *transaction;
		struct strbuf err = STRBUF_INIT;
		char *msg;

		if (forcing)
			msg = xstrfmt("branch: Reset to %s", start_name);
		else
			msg = xstrfmt("branch: Created from %s", start_name);

		transaction = ref_transaction_begin(&err);
		if (!transaction ||
		    ref_transaction_update(transaction, ref.buf,
					   sha1, forcing ? NULL : null_sha1,
					   0, msg, &err) ||
		    ref_transaction_commit(transaction, &err))
			die("%s", err.buf);
		ref_transaction_free(transaction);
		strbuf_release(&err);
		free(msg);
	}

	if (real_ref && track)
		setup_tracking(ref.buf + 11, real_ref, track, quiet);

	strbuf_release(&ref);
