int cmd_merge_file(int argc, const char **argv, const char *prefix)
{
	const char *names[3] = { NULL, NULL, NULL };
	mmfile_t mmfs[3];
	mmbuffer_t result = {NULL, 0};
	xmparam_t xmp = {{0}};
	int ret = 0, i = 0, to_stdout = 0;
	int quiet = 0;
	int prefixlen = 0;
	struct option options[] = {
		OPT_BOOL('p', "stdout", &to_stdout, N_("send results to standard output")),
		OPT_SET_INT(0, "diff3", &xmp.style, N_("use a diff3 based merge"), XDL_MERGE_DIFF3),
		OPT_SET_INT(0, "ours", &xmp.favor, N_("for conflicts, use our version"),
			    XDL_MERGE_FAVOR_OURS),
		OPT_SET_INT(0, "theirs", &xmp.favor, N_("for conflicts, use their version"),
			    XDL_MERGE_FAVOR_THEIRS),
		OPT_SET_INT(0, "union", &xmp.favor, N_("for conflicts, use a union version"),
			    XDL_MERGE_FAVOR_UNION),
		OPT_INTEGER(0, "marker-size", &xmp.marker_size,
			    N_("for conflicts, use this marker size")),
		OPT__QUIET(&quiet, N_("do not warn about conflicts")),
		OPT_CALLBACK('L', NULL, names, N_("name"),
			     N_("set labels for file1/orig_file/file2"), &label_cb),
		OPT_END(),
	};

	xmp.level = XDL_MERGE_ZEALOUS_ALNUM;
	xmp.style = 0;
	xmp.favor = 0;

	if (startup_info->have_repository) {
		/* Read the configuration file */
		git_config(git_xmerge_config, NULL);
		if (0 <= git_xmerge_style)
			xmp.style = git_xmerge_style;
	}

	argc = parse_options(argc, argv, prefix, options, merge_file_usage, 0);
	if (argc != 3)
		usage_with_options(merge_file_usage, options);
	if (quiet) {
		if (!freopen("/dev/null", "w", stderr))
			return error("failed to redirect stderr to /dev/null: "
				     "%s", strerror(errno));
	}

	if (prefix)
		prefixlen = strlen(prefix);

	for (i = 0; i < 3; i++) {
		const char *fname = prefix_filename(prefix, prefixlen, argv[i]);
		if (!names[i])
			names[i] = argv[i];
		if (read_mmfile(mmfs + i, fname))
			return -1;
		if (buffer_is_binary(mmfs[i].ptr, mmfs[i].size))
			return error("Cannot merge binary files: %s",
					argv[i]);
	}

	xmp.ancestor = names[1];
	xmp.file1 = names[0];
	xmp.file2 = names[2];
	ret = xdl_merge(mmfs + 1, mmfs + 0, mmfs + 2, &xmp, &result);

	for (i = 0; i < 3; i++)
		free(mmfs[i].ptr);

	if (ret >= 0) {
		const char *filename = argv[0];
		const char *fpath = prefix_filename(prefix, prefixlen, argv[0]);
		FILE *f = to_stdout ? stdout : fopen(fpath, "wb");

		if (!f)
			ret = error("Could not open %s for writing", filename);
		else if (result.size &&
			 fwrite(result.ptr, result.size, 1, f) != 1)
			ret = error("Could not write to %s", filename);
		else if (fclose(f))
			ret = error("Could not close %s", filename);
		free(result.ptr);
	}

	return ret;
}