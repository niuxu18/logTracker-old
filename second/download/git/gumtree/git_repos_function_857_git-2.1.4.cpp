static int handle_revision_pseudo_opt(const char *submodule,
				struct rev_info *revs,
				int argc, const char **argv, int *flags)
{
	const char *arg = argv[0];
	const char *optarg;
	int argcount;

	/*
	 * NOTE!
	 *
	 * Commands like "git shortlog" will not accept the options below
	 * unless parse_revision_opt queues them (as opposed to erroring
	 * out).
	 *
	 * When implementing your new pseudo-option, remember to
	 * register it in the list at the top of handle_revision_opt.
	 */
	if (!strcmp(arg, "--all")) {
		handle_refs(submodule, revs, *flags, for_each_ref_submodule);
		handle_refs(submodule, revs, *flags, head_ref_submodule);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (!strcmp(arg, "--branches")) {
		handle_refs(submodule, revs, *flags, for_each_branch_ref_submodule);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (!strcmp(arg, "--bisect")) {
		handle_refs(submodule, revs, *flags, for_each_bad_bisect_ref);
		handle_refs(submodule, revs, *flags ^ (UNINTERESTING | BOTTOM), for_each_good_bisect_ref);
		revs->bisect = 1;
	} else if (!strcmp(arg, "--tags")) {
		handle_refs(submodule, revs, *flags, for_each_tag_ref_submodule);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (!strcmp(arg, "--remotes")) {
		handle_refs(submodule, revs, *flags, for_each_remote_ref_submodule);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if ((argcount = parse_long_opt("glob", argv, &optarg))) {
		struct all_refs_cb cb;
		init_all_refs_cb(&cb, revs, *flags);
		for_each_glob_ref(handle_one_ref, optarg, &cb);
		clear_ref_exclusion(&revs->ref_excludes);
		return argcount;
	} else if ((argcount = parse_long_opt("exclude", argv, &optarg))) {
		add_ref_exclusion(&revs->ref_excludes, optarg);
		return argcount;
	} else if (starts_with(arg, "--branches=")) {
		struct all_refs_cb cb;
		init_all_refs_cb(&cb, revs, *flags);
		for_each_glob_ref_in(handle_one_ref, arg + 11, "refs/heads/", &cb);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (starts_with(arg, "--tags=")) {
		struct all_refs_cb cb;
		init_all_refs_cb(&cb, revs, *flags);
		for_each_glob_ref_in(handle_one_ref, arg + 7, "refs/tags/", &cb);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (starts_with(arg, "--remotes=")) {
		struct all_refs_cb cb;
		init_all_refs_cb(&cb, revs, *flags);
		for_each_glob_ref_in(handle_one_ref, arg + 10, "refs/remotes/", &cb);
		clear_ref_exclusion(&revs->ref_excludes);
	} else if (!strcmp(arg, "--reflog")) {
		handle_reflog(revs, *flags);
	} else if (!strcmp(arg, "--not")) {
		*flags ^= UNINTERESTING | BOTTOM;
	} else if (!strcmp(arg, "--no-walk")) {
		revs->no_walk = REVISION_WALK_NO_WALK_SORTED;
	} else if (starts_with(arg, "--no-walk=")) {
		/*
		 * Detached form ("--no-walk X" as opposed to "--no-walk=X")
		 * not allowed, since the argument is optional.
		 */
		if (!strcmp(arg + 10, "sorted"))
			revs->no_walk = REVISION_WALK_NO_WALK_SORTED;
		else if (!strcmp(arg + 10, "unsorted"))
			revs->no_walk = REVISION_WALK_NO_WALK_UNSORTED;
		else
			return error("invalid argument to --no-walk");
	} else if (!strcmp(arg, "--do-walk")) {
		revs->no_walk = 0;
	} else {
		return 0;
	}

	return 1;
}