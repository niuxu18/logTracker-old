	if (!ref_name_cnt) {
		fprintf(stderr, "No revs to be shown.\n");
		exit(0);
	}

	for (num_rev = 0; ref_name[num_rev]; num_rev++) {
		struct object_id revkey;
		unsigned int flag = 1u << (num_rev + REV_SHIFT);

		if (MAX_REVS <= num_rev)
			die("cannot handle more than %d revs.", MAX_REVS);
		if (get_sha1(ref_name[num_rev], revkey.hash))
			die("'%s' is not a valid ref.", ref_name[num_rev]);
		commit = lookup_commit_reference(revkey.hash);
		if (!commit)
			die("cannot find commit %s (%s)",
			    ref_name[num_rev], oid_to_hex(&revkey));
		parse_commit(commit);
		mark_seen(commit, &seen);

		/* rev#0 uses bit REV_SHIFT, rev#1 uses bit REV_SHIFT+1,
		 * and so on.  REV_SHIFT bits from bit 0 are used for
		 * internal bookkeeping.
