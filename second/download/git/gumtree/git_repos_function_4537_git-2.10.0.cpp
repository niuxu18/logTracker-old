int cmd_bisect__helper(int argc, const char **argv, const char *prefix)
{
	int next_all = 0;
	int no_checkout = 0;
	struct option options[] = {
		OPT_BOOL(0, "next-all", &next_all,
			 N_("perform 'git bisect next'")),
		OPT_BOOL(0, "no-checkout", &no_checkout,
			 N_("update BISECT_HEAD instead of checking out the current commit")),
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, options,
			     git_bisect_helper_usage, 0);

	if (!next_all)
		usage_with_options(git_bisect_helper_usage, options);

	/* next-all */
	return bisect_next_all(prefix, no_checkout);
}