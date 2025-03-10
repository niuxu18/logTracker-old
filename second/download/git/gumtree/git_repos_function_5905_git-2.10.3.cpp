static int lock_raw_ref(const char *refname, int mustexist,
			const struct string_list *extras,
			const struct string_list *skip,
			struct ref_lock **lock_p,
			struct strbuf *referent,
			unsigned int *type,
			struct strbuf *err)
{
	struct ref_lock *lock;
	struct strbuf ref_file = STRBUF_INIT;
	int attempts_remaining = 3;
	int ret = TRANSACTION_GENERIC_ERROR;

	assert(err);
	*type = 0;

	/* First lock the file so it can't change out from under us. */

	*lock_p = lock = xcalloc(1, sizeof(*lock));

	lock->ref_name = xstrdup(refname);
	strbuf_git_path(&ref_file, "%s", refname);

retry:
	switch (safe_create_leading_directories(ref_file.buf)) {
	case SCLD_OK:
		break; /* success */
	case SCLD_EXISTS:
		/*
		 * Suppose refname is "refs/foo/bar". We just failed
		 * to create the containing directory, "refs/foo",
		 * because there was a non-directory in the way. This
		 * indicates a D/F conflict, probably because of
		 * another reference such as "refs/foo". There is no
		 * reason to expect this error to be transitory.
		 */
		if (verify_refname_available(refname, extras, skip, err)) {
			if (mustexist) {
				/*
				 * To the user the relevant error is
				 * that the "mustexist" reference is
				 * missing:
				 */
				strbuf_reset(err);
				strbuf_addf(err, "unable to resolve reference '%s'",
					    refname);
			} else {
				/*
				 * The error message set by
				 * verify_refname_available_dir() is OK.
				 */
				ret = TRANSACTION_NAME_CONFLICT;
			}
		} else {
			/*
			 * The file that is in the way isn't a loose
			 * reference. Report it as a low-level
			 * failure.
			 */
			strbuf_addf(err, "unable to create lock file %s.lock; "
				    "non-directory in the way",
				    ref_file.buf);
		}
		goto error_return;
	case SCLD_VANISHED:
		/* Maybe another process was tidying up. Try again. */
		if (--attempts_remaining > 0)
			goto retry;
		/* fall through */
	default:
		strbuf_addf(err, "unable to create directory for %s",
			    ref_file.buf);
		goto error_return;
	}

	if (!lock->lk)
		lock->lk = xcalloc(1, sizeof(struct lock_file));

	if (hold_lock_file_for_update(lock->lk, ref_file.buf, LOCK_NO_DEREF) < 0) {
		if (errno == ENOENT && --attempts_remaining > 0) {
			/*
			 * Maybe somebody just deleted one of the
			 * directories leading to ref_file.  Try
			 * again:
			 */
			goto retry;
		} else {
			unable_to_lock_message(ref_file.buf, errno, err);
			goto error_return;
		}
	}

	/*
	 * Now we hold the lock and can read the reference without
	 * fear that its value will change.
	 */

	if (read_raw_ref(refname, lock->old_oid.hash, referent, type)) {
		if (errno == ENOENT) {
			if (mustexist) {
				/* Garden variety missing reference. */
				strbuf_addf(err, "unable to resolve reference '%s'",
					    refname);
				goto error_return;
			} else {
				/*
				 * Reference is missing, but that's OK. We
				 * know that there is not a conflict with
				 * another loose reference because
				 * (supposing that we are trying to lock
				 * reference "refs/foo/bar"):
				 *
				 * - We were successfully able to create
				 *   the lockfile refs/foo/bar.lock, so we
				 *   know there cannot be a loose reference
				 *   named "refs/foo".
				 *
				 * - We got ENOENT and not EISDIR, so we
				 *   know that there cannot be a loose
				 *   reference named "refs/foo/bar/baz".
				 */
			}
		} else if (errno == EISDIR) {
			/*
			 * There is a directory in the way. It might have
			 * contained references that have been deleted. If
			 * we don't require that the reference already
			 * exists, try to remove the directory so that it
			 * doesn't cause trouble when we want to rename the
			 * lockfile into place later.
			 */
			if (mustexist) {
				/* Garden variety missing reference. */
				strbuf_addf(err, "unable to resolve reference '%s'",
					    refname);
				goto error_return;
			} else if (remove_dir_recursively(&ref_file,
							  REMOVE_DIR_EMPTY_ONLY)) {
				if (verify_refname_available_dir(
						    refname, extras, skip,
						    get_loose_refs(&ref_cache),
						    err)) {
					/*
					 * The error message set by
					 * verify_refname_available() is OK.
					 */
					ret = TRANSACTION_NAME_CONFLICT;
					goto error_return;
				} else {
					/*
					 * We can't delete the directory,
					 * but we also don't know of any
					 * references that it should
					 * contain.
					 */
					strbuf_addf(err, "there is a non-empty directory '%s' "
						    "blocking reference '%s'",
						    ref_file.buf, refname);
					goto error_return;
				}
			}
		} else if (errno == EINVAL && (*type & REF_ISBROKEN)) {
			strbuf_addf(err, "unable to resolve reference '%s': "
				    "reference broken", refname);
			goto error_return;
		} else {
			strbuf_addf(err, "unable to resolve reference '%s': %s",
				    refname, strerror(errno));
			goto error_return;
		}

		/*
		 * If the ref did not exist and we are creating it,
		 * make sure there is no existing packed ref whose
		 * name begins with our refname, nor a packed ref
		 * whose name is a proper prefix of our refname.
		 */
		if (verify_refname_available_dir(
				    refname, extras, skip,
				    get_packed_refs(&ref_cache),
				    err)) {
			goto error_return;
		}
	}

	ret = 0;
	goto out;

error_return:
	unlock_ref(lock);
	*lock_p = NULL;

out:
	strbuf_release(&ref_file);
	return ret;
}