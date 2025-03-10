int cmd_stripspace(int argc, const char **argv, const char *prefix)
{
	struct strbuf buf = STRBUF_INIT;
	enum stripspace_mode mode = STRIP_DEFAULT;

	const struct option options[] = {
		OPT_CMDMODE('s', "strip-comments", &mode,
			    N_("skip and remove all lines starting with comment character"),
			    STRIP_COMMENTS),
		OPT_CMDMODE('c', "comment-lines", &mode,
			    N_("prepend comment character and space to each line"),
			    COMMENT_LINES),
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, options, stripspace_usage, 0);
	if (argc)
		usage_with_options(stripspace_usage, options);

	if (mode == STRIP_COMMENTS || mode == COMMENT_LINES) {
		setup_git_directory_gently(NULL);
		git_config(git_default_config, NULL);
	}

	if (strbuf_read(&buf, 0, 1024) < 0)
		die_errno("could not read the input");

	if (mode == STRIP_DEFAULT || mode == STRIP_COMMENTS)
		strbuf_stripspace(&buf, mode == STRIP_COMMENTS);
	else
		comment_lines(&buf);

	write_or_die(1, buf.buf, buf.len);
	strbuf_release(&buf);
	return 0;
}