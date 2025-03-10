
	series_dir_buf = xstrdup(*paths);
	series_dir = dirname(series_dir_buf);

	fp = fopen(*paths, "r");
	if (!fp)
		return error_errno(_("could not open '%s' for reading"), *paths);

	while (!strbuf_getline_lf(&sb, fp)) {
		if (*sb.buf == '#')
			continue; /* skip comment lines */

		argv_array_push(&patches, mkpath("%s/%s", series_dir, sb.buf));
