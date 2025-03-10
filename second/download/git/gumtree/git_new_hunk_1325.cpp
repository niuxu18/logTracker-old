	}

	strbuf_addf(&msg, "notes: Merged notes from %s into %s",
		    remote_ref.buf, default_notes_ref());
	strbuf_add(&(o.commit_msg), msg.buf + 7, msg.len - 7); /* skip "notes: " */

	result = notes_merge(&o, t, &result_oid);

	if (result >= 0) /* Merge resulted (trivially) in result_oid */
		/* Update default notes ref with new commit */
		update_ref(msg.buf, default_notes_ref(), result_oid.hash, NULL,
			   0, UPDATE_REFS_DIE_ON_ERR);
	else { /* Merge has unresolved conflicts */
		const struct worktree *wt;
		/* Update .git/NOTES_MERGE_PARTIAL with partial merge result */
		update_ref(msg.buf, "NOTES_MERGE_PARTIAL", result_oid.hash, NULL,
			   0, UPDATE_REFS_DIE_ON_ERR);
		/* Store ref-to-be-updated into .git/NOTES_MERGE_REF */
		wt = find_shared_symref("NOTES_MERGE_REF", default_notes_ref());
		if (wt)
			die(_("a notes merge into %s is already in-progress at %s"),
			    default_notes_ref(), wt->path);
