static int fall_back_threeway(const struct am_state *state, const char *index_path)
{
	unsigned char orig_tree[GIT_SHA1_RAWSZ], his_tree[GIT_SHA1_RAWSZ],
		      our_tree[GIT_SHA1_RAWSZ];

	if (get_sha1("HEAD", our_tree) < 0)
		hashcpy(our_tree, EMPTY_TREE_SHA1_BIN);

	if (build_fake_ancestor(state, index_path))
		return error("could not build fake ancestor");

	discard_cache();
	read_cache_from(index_path);

	if (write_index_as_tree(orig_tree, &the_index, index_path, 0, NULL))
		return error(_("Repository lacks necessary blobs to fall back on 3-way merge."));

	say(state, stdout, _("Using index info to reconstruct a base tree..."));

	if (!state->quiet) {
		/*
		 * List paths that needed 3-way fallback, so that the user can
		 * review them with extra care to spot mismerges.
		 */
		struct rev_info rev_info;
		const char *diff_filter_str = "--diff-filter=AM";

		init_revisions(&rev_info, NULL);
		rev_info.diffopt.output_format = DIFF_FORMAT_NAME_STATUS;
		diff_opt_parse(&rev_info.diffopt, &diff_filter_str, 1);
		add_pending_sha1(&rev_info, "HEAD", our_tree, 0);
		diff_setup_done(&rev_info.diffopt);
		run_diff_index(&rev_info, 1);
	}

	if (run_apply(state, index_path))
		return error(_("Did you hand edit your patch?\n"
				"It does not apply to blobs recorded in its index."));

	if (write_index_as_tree(his_tree, &the_index, index_path, 0, NULL))
		return error("could not write tree");

	say(state, stdout, _("Falling back to patching base and 3-way merge..."));

	discard_cache();
	read_cache();

	/*
	 * This is not so wrong. Depending on which base we picked, orig_tree
	 * may be wildly different from ours, but his_tree has the same set of
	 * wildly different changes in parts the patch did not touch, so
	 * recursive ends up canceling them, saying that we reverted all those
	 * changes.
	 */

	if (run_fallback_merge_recursive(state, orig_tree, our_tree, his_tree)) {
		rerere(state->allow_rerere_autoupdate);
		return error(_("Failed to merge in the changes."));
	}

	return 0;
}