static void copy_alternates(struct strbuf *src, struct strbuf *dst,
			    const char *src_repo)
{
	/*
	 * Read from the source objects/info/alternates file
	 * and copy the entries to corresponding file in the
	 * destination repository with add_to_alternates_file().
	 * Both src and dst have "$path/objects/info/alternates".
	 *
	 * Instead of copying bit-for-bit from the original,
	 * we need to append to existing one so that the already
	 * created entry via "clone -s" is not lost, and also
	 * to turn entries with paths relative to the original
	 * absolute, so that they can be used in the new repository.
	 */
	FILE *in = fopen(src->buf, "r");
	struct strbuf line = STRBUF_INIT;

	while (strbuf_getline(&line, in, '\n') != EOF) {
		char *abs_path, abs_buf[PATH_MAX];
		if (!line.len || line.buf[0] == '#')
			continue;
		if (is_absolute_path(line.buf)) {
			add_to_alternates_file(line.buf);
			continue;
		}
		abs_path = mkpath("%s/objects/%s", src_repo, line.buf);
		normalize_path_copy(abs_buf, abs_path);
		add_to_alternates_file(abs_buf);
	}
	strbuf_release(&line);
	fclose(in);
}