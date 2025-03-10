	free(to_free);

	return 0;
}

typedef int (*each_tag_name_fn)(const char *name, const char *ref,
				const unsigned char *sha1, const void *cb_data);

static int for_each_tag_name(const char **argv, each_tag_name_fn fn,
			     const void *cb_data)
{
	const char **p;
	struct strbuf ref = STRBUF_INIT;
	int had_error = 0;
	unsigned char sha1[20];

	for (p = argv; *p; p++) {
		strbuf_reset(&ref);
		strbuf_addf(&ref, "refs/tags/%s", *p);
		if (read_ref(ref.buf, sha1)) {
			error(_("tag '%s' not found."), *p);
			had_error = 1;
			continue;
		}
		if (fn(*p, ref.buf, sha1, cb_data))
			had_error = 1;
	}
	strbuf_release(&ref);
	return had_error;
}

static int delete_tag(const char *name, const char *ref,
		      const unsigned char *sha1, const void *cb_data)
{
	if (delete_ref(NULL, ref, sha1, 0))
		return 1;
	printf(_("Deleted tag '%s' (was %s)\n"), name, find_unique_abbrev(sha1, DEFAULT_ABBREV));
	return 0;
}

static int verify_tag(const char *name, const char *ref,
		      const unsigned char *sha1, const void *cb_data)
{
	int flags;
	const char *fmt_pretty = cb_data;
	flags = GPG_VERIFY_VERBOSE;

	if (fmt_pretty)
		flags = GPG_VERIFY_OMIT_STATUS;

	if (gpg_verify_tag(sha1, name, flags))
		return -1;

	if (fmt_pretty)
		pretty_print_ref(name, sha1, fmt_pretty);

	return 0;
}

static int do_sign(struct strbuf *buffer)
{
