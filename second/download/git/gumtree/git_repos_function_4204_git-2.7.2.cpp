int cmd_update_server_info(int argc, const char **argv, const char *prefix)
{
	int force = 0;
	struct option options[] = {
		OPT__FORCE(&force, N_("update the info files from scratch")),
		OPT_END()
	};

	git_config(git_default_config, NULL);
	argc = parse_options(argc, argv, prefix, options,
			     update_server_info_usage, 0);
	if (argc > 0)
		usage_with_options(update_server_info_usage, options);

	return !!update_server_info(force);
}