	struct commit *commit;
	int url_len, i, rc = 0;
	struct strbuf note = STRBUF_INIT;
	const char *what, *kind;
	struct ref *rm;
	char *url;
	const char *filename = dry_run ? "/dev/null" : git_path_fetch_head();
	int want_status;

	fp = fopen(filename, "a");
	if (!fp)
		return error(_("cannot open %s: %s\n"), filename, strerror(errno));

