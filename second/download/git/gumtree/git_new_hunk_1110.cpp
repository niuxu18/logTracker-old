	return xstrfmt("%.*s%.*s", (int)prefix_len, url,
		       (int)anon_len, anon_part);
literal_copy:
	return xstrdup(url);
}

static void read_alternate_refs(const char *path,
				alternate_ref_fn *cb,
				void *data)
{
	struct child_process cmd = CHILD_PROCESS_INIT;
	struct strbuf line = STRBUF_INIT;
	FILE *fh;

	cmd.git_cmd = 1;
	argv_array_pushf(&cmd.args, "--git-dir=%s", path);
	argv_array_push(&cmd.args, "for-each-ref");
	argv_array_push(&cmd.args, "--format=%(objectname) %(refname)");
	cmd.env = local_repo_env;
	cmd.out = -1;

	if (start_command(&cmd))
		return;

	fh = xfdopen(cmd.out, "r");
	while (strbuf_getline_lf(&line, fh) != EOF) {
		struct object_id oid;

		if (get_oid_hex(line.buf, &oid) ||
		    line.buf[GIT_SHA1_HEXSZ] != ' ') {
			warning("invalid line while parsing alternate refs: %s",
				line.buf);
			break;
		}

		cb(line.buf + GIT_SHA1_HEXSZ + 1, &oid, data);
	}

	fclose(fh);
	finish_command(&cmd);
}

struct alternate_refs_data {
	alternate_ref_fn *fn;
	void *data;
};

static int refs_from_alternate_cb(struct alternate_object_database *e,
				  void *data)
{
	struct strbuf path = STRBUF_INIT;
	size_t base_len;
	struct alternate_refs_data *cb = data;

	if (!strbuf_realpath(&path, e->path, 0))
		goto out;
	if (!strbuf_strip_suffix(&path, "/objects"))
		goto out;
	base_len = path.len;

	/* Is this a git repository with refs? */
	strbuf_addstr(&path, "/refs");
	if (!is_directory(path.buf))
		goto out;
	strbuf_setlen(&path, base_len);

	read_alternate_refs(path.buf, cb->fn, cb->data);

out:
	strbuf_release(&path);
	return 0;
}

void for_each_alternate_ref(alternate_ref_fn fn, void *data)
{
	struct alternate_refs_data cb;
