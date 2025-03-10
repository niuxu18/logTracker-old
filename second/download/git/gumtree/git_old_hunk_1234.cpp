	if (userdiff_config(var, value) < 0)
		return -1;

	return git_default_config(var, value, cb);
}

static void verify_working_tree_path(struct commit *work_tree, const char *path)
{
	struct commit_list *parents;
	int pos;

	for (parents = work_tree->parents; parents; parents = parents->next) {
		const struct object_id *commit_oid = &parents->item->object.oid;
		struct object_id blob_oid;
		unsigned mode;

		if (!get_tree_entry(commit_oid->hash, path, blob_oid.hash, &mode) &&
		    sha1_object_info(blob_oid.hash, NULL) == OBJ_BLOB)
			return;
	}

	pos = cache_name_pos(path, strlen(path));
	if (pos >= 0)
		; /* path is in the index */
	else if (-1 - pos < active_nr &&
		 !strcmp(active_cache[-1 - pos]->name, path))
		; /* path is in the index, unmerged */
	else
		die("no such path '%s' in HEAD", path);
}

static struct commit_list **append_parent(struct commit_list **tail, const struct object_id *oid)
{
	struct commit *parent;

	parent = lookup_commit_reference(oid->hash);
	if (!parent)
		die("no such commit %s", oid_to_hex(oid));
	return &commit_list_insert(parent, tail)->next;
}

static void append_merge_parents(struct commit_list **tail)
{
	int merge_head;
	struct strbuf line = STRBUF_INIT;

	merge_head = open(git_path_merge_head(), O_RDONLY);
	if (merge_head < 0) {
		if (errno == ENOENT)
			return;
		die("cannot open '%s' for reading", git_path_merge_head());
	}

	while (!strbuf_getwholeline_fd(&line, merge_head, '\n')) {
		struct object_id oid;
		if (line.len < GIT_SHA1_HEXSZ || get_oid_hex(line.buf, &oid))
			die("unknown line in '%s': %s", git_path_merge_head(), line.buf);
		tail = append_parent(tail, &oid);
	}
	close(merge_head);
	strbuf_release(&line);
}

/*
 * This isn't as simple as passing sb->buf and sb->len, because we
 * want to transfer ownership of the buffer to the commit (so we
 * must use detach).
 */
static void set_commit_buffer_from_strbuf(struct commit *c, struct strbuf *sb)
{
	size_t len;
	void *buf = strbuf_detach(sb, &len);
	set_commit_buffer(c, buf, len);
}

/*
 * Prepare a dummy commit that represents the work tree (or staged) item.
 * Note that annotating work tree item never works in the reverse.
 */
static struct commit *fake_working_tree_commit(struct diff_options *opt,
					       const char *path,
					       const char *contents_from)
{
	struct commit *commit;
	struct origin *origin;
	struct commit_list **parent_tail, *parent;
	struct object_id head_oid;
	struct strbuf buf = STRBUF_INIT;
	const char *ident;
	time_t now;
	int size, len;
	struct cache_entry *ce;
	unsigned mode;
	struct strbuf msg = STRBUF_INIT;

	read_cache();
	time(&now);
	commit = alloc_commit_node();
	commit->object.parsed = 1;
	commit->date = now;
	parent_tail = &commit->parents;

	if (!resolve_ref_unsafe("HEAD", RESOLVE_REF_READING, head_oid.hash, NULL))
		die("no such ref: HEAD");

	parent_tail = append_parent(parent_tail, &head_oid);
	append_merge_parents(parent_tail);
	verify_working_tree_path(commit, path);

	origin = make_origin(commit, path);

	ident = fmt_ident("Not Committed Yet", "not.committed.yet", NULL, 0);
	strbuf_addstr(&msg, "tree 0000000000000000000000000000000000000000\n");
	for (parent = commit->parents; parent; parent = parent->next)
		strbuf_addf(&msg, "parent %s\n",
			    oid_to_hex(&parent->item->object.oid));
	strbuf_addf(&msg,
		    "author %s\n"
		    "committer %s\n\n"
		    "Version of %s from %s\n",
		    ident, ident, path,
		    (!contents_from ? path :
		     (!strcmp(contents_from, "-") ? "standard input" : contents_from)));
	set_commit_buffer_from_strbuf(commit, &msg);

	if (!contents_from || strcmp("-", contents_from)) {
		struct stat st;
		const char *read_from;
		char *buf_ptr;
		unsigned long buf_len;

		if (contents_from) {
			if (stat(contents_from, &st) < 0)
				die_errno("Cannot stat '%s'", contents_from);
			read_from = contents_from;
		}
		else {
			if (lstat(path, &st) < 0)
				die_errno("Cannot lstat '%s'", path);
			read_from = path;
		}
		mode = canon_mode(st.st_mode);

		switch (st.st_mode & S_IFMT) {
		case S_IFREG:
			if (DIFF_OPT_TST(opt, ALLOW_TEXTCONV) &&
			    textconv_object(read_from, mode, &null_oid, 0, &buf_ptr, &buf_len))
				strbuf_attach(&buf, buf_ptr, buf_len, buf_len + 1);
			else if (strbuf_read_file(&buf, read_from, st.st_size) != st.st_size)
				die_errno("cannot open or read '%s'", read_from);
			break;
		case S_IFLNK:
			if (strbuf_readlink(&buf, read_from, st.st_size) < 0)
				die_errno("cannot readlink '%s'", read_from);
			break;
		default:
			die("unsupported file type %s", read_from);
		}
	}
	else {
		/* Reading from stdin */
		mode = 0;
		if (strbuf_read(&buf, 0, 0) < 0)
			die_errno("failed to read from stdin");
	}
	convert_to_git(path, buf.buf, buf.len, &buf, 0);
	origin->file.ptr = buf.buf;
	origin->file.size = buf.len;
	pretend_sha1_file(buf.buf, buf.len, OBJ_BLOB, origin->blob_oid.hash);

	/*
	 * Read the current index, replace the path entry with
	 * origin->blob_sha1 without mucking with its mode or type
	 * bits; we are not going to write this index out -- we just
	 * want to run "diff-index --cached".
	 */
	discard_cache();
	read_cache();

	len = strlen(path);
	if (!mode) {
		int pos = cache_name_pos(path, len);
		if (0 <= pos)
			mode = active_cache[pos]->ce_mode;
		else
			/* Let's not bother reading from HEAD tree */
			mode = S_IFREG | 0644;
	}
	size = cache_entry_size(len);
	ce = xcalloc(1, size);
	oidcpy(&ce->oid, &origin->blob_oid);
	memcpy(ce->name, path, len);
	ce->ce_flags = create_ce_flags(0);
	ce->ce_namelen = len;
	ce->ce_mode = create_ce_mode(mode);
	add_cache_entry(ce, ADD_CACHE_OK_TO_ADD|ADD_CACHE_OK_TO_REPLACE);

	cache_tree_invalidate_path(&the_index, path);

	return commit;
}

static struct commit *find_single_final(struct rev_info *revs,
					const char **name_p)
{
	int i;
	struct commit *found = NULL;
	const char *name = NULL;

	for (i = 0; i < revs->pending.nr; i++) {
		struct object *obj = revs->pending.objects[i].item;
		if (obj->flags & UNINTERESTING)
			continue;
		obj = deref_tag(obj, NULL, 0);
		if (obj->type != OBJ_COMMIT)
			die("Non commit %s?", revs->pending.objects[i].name);
		if (found)
			die("More than one commit to dig from %s and %s?",
			    revs->pending.objects[i].name, name);
		found = (struct commit *)obj;
		name = revs->pending.objects[i].name;
	}
	if (name_p)
		*name_p = name;
	return found;
}

static char *prepare_final(struct scoreboard *sb)
{
	const char *name;
	sb->final = find_single_final(sb->revs, &name);
	return xstrdup_or_null(name);
}

static const char *dwim_reverse_initial(struct scoreboard *sb)
{
	/*
	 * DWIM "git blame --reverse ONE -- PATH" as
	 * "git blame --reverse ONE..HEAD -- PATH" but only do so
	 * when it makes sense.
	 */
	struct object *obj;
	struct commit *head_commit;
	unsigned char head_sha1[20];

	if (sb->revs->pending.nr != 1)
		return NULL;

	/* Is that sole rev a committish? */
	obj = sb->revs->pending.objects[0].item;
	obj = deref_tag(obj, NULL, 0);
	if (obj->type != OBJ_COMMIT)
		return NULL;

	/* Do we have HEAD? */
	if (!resolve_ref_unsafe("HEAD", RESOLVE_REF_READING, head_sha1, NULL))
		return NULL;
	head_commit = lookup_commit_reference_gently(head_sha1, 1);
	if (!head_commit)
		return NULL;

	/* Turn "ONE" into "ONE..HEAD" then */
	obj->flags |= UNINTERESTING;
	add_pending_object(sb->revs, &head_commit->object, "HEAD");

	sb->final = (struct commit *)obj;
	return sb->revs->pending.objects[0].name;
}

static char *prepare_initial(struct scoreboard *sb)
{
	int i;
	const char *final_commit_name = NULL;
	struct rev_info *revs = sb->revs;

	/*
	 * There must be one and only one negative commit, and it must be
	 * the boundary.
	 */
	for (i = 0; i < revs->pending.nr; i++) {
		struct object *obj = revs->pending.objects[i].item;
		if (!(obj->flags & UNINTERESTING))
			continue;
		obj = deref_tag(obj, NULL, 0);
		if (obj->type != OBJ_COMMIT)
			die("Non commit %s?", revs->pending.objects[i].name);
		if (sb->final)
			die("More than one commit to dig up from, %s and %s?",
			    revs->pending.objects[i].name,
			    final_commit_name);
		sb->final = (struct commit *) obj;
		final_commit_name = revs->pending.objects[i].name;
	}

	if (!final_commit_name)
		final_commit_name = dwim_reverse_initial(sb);
	if (!final_commit_name)
		die("No commit to dig up from?");
	return xstrdup(final_commit_name);
}

static int blame_copy_callback(const struct option *option, const char *arg, int unset)
{
	int *opt = option->value;

	/*
	 * -C enables copy from removed files;
