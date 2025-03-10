int cmd_symbolic_ref(int argc, const char **argv, const char *prefix)
{
	int quiet = 0, delete = 0, shorten = 0, ret = 0;
	const char *msg = NULL;
	struct option options[] = {
		OPT__QUIET(&quiet,
			N_("suppress error message for non-symbolic (detached) refs")),
		OPT_BOOL('d', "delete", &delete, N_("delete symbolic ref")),
		OPT_BOOL(0, "short", &shorten, N_("shorten ref output")),
		OPT_STRING('m', NULL, &msg, N_("reason"), N_("reason of the update")),
		OPT_END(),
	};

	git_config(git_default_config, NULL);
	argc = parse_options(argc, argv, prefix, options,
			     git_symbolic_ref_usage, 0);
	if (msg && !*msg)
		die("Refusing to perform update with empty message");

	if (delete) {
		if (argc != 1)
			usage_with_options(git_symbolic_ref_usage, options);
		ret = check_symref(argv[0], 1, 0, 0);
		if (ret)
			die("Cannot delete %s, not a symbolic ref", argv[0]);
		if (!strcmp(argv[0], "HEAD"))
			die("deleting '%s' is not allowed", argv[0]);
		return delete_ref(argv[0], NULL, REF_NODEREF);
	}

	switch (argc) {
	case 1:
		ret = check_symref(argv[0], quiet, shorten, 1);
		break;
	case 2:
		if (!strcmp(argv[0], "HEAD") &&
		    !starts_with(argv[1], "refs/"))
			die("Refusing to point HEAD outside of refs/");
		ret = !!create_symref(argv[0], argv[1], msg);
		break;
	default:
		usage_with_options(git_symbolic_ref_usage, options);
	}
	return ret;
}