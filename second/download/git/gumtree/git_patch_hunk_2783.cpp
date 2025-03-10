 
 	free(name);
 
 	return(ret);
 }
 
-struct ref_item {
-	char *name;
-	char *dest;
-	unsigned int kind, width;
-	struct commit *commit;
-	int ignore;
-};
-
-struct ref_list {
-	struct rev_info revs;
-	int index, alloc, maxwidth, verbose, abbrev;
-	struct ref_item *list;
-	struct commit_list *with_commit;
-	int kinds;
-};
-
-static char *resolve_symref(const char *src, const char *prefix)
-{
-	unsigned char sha1[20];
-	int flag;
-	const char *dst;
-
-	dst = resolve_ref_unsafe(src, 0, sha1, &flag);
-	if (!(dst && (flag & REF_ISSYMREF)))
-		return NULL;
-	if (prefix)
-		skip_prefix(dst, prefix, &dst);
-	return xstrdup(dst);
-}
-
-struct append_ref_cb {
-	struct ref_list *ref_list;
-	const char **pattern;
-	int ret;
-};
-
-static int match_patterns(const char **pattern, const char *refname)
-{
-	if (!*pattern)
-		return 1; /* no pattern always matches */
-	while (*pattern) {
-		if (!wildmatch(*pattern, refname, 0, NULL))
-			return 1;
-		pattern++;
-	}
-	return 0;
-}
-
-static int append_ref(const char *refname, const struct object_id *oid, int flags, void *cb_data)
-{
-	struct append_ref_cb *cb = (struct append_ref_cb *)(cb_data);
-	struct ref_list *ref_list = cb->ref_list;
-	struct ref_item *newitem;
-	struct commit *commit;
-	int kind, i;
-	const char *prefix, *orig_refname = refname;
-
-	static struct {
-		int kind;
-		const char *prefix;
-	} ref_kind[] = {
-		{ REF_LOCAL_BRANCH, "refs/heads/" },
-		{ REF_REMOTE_BRANCH, "refs/remotes/" },
-	};
-
-	/* Detect kind */
-	for (i = 0; i < ARRAY_SIZE(ref_kind); i++) {
-		prefix = ref_kind[i].prefix;
-		if (skip_prefix(refname, prefix, &refname)) {
-			kind = ref_kind[i].kind;
-			break;
-		}
-	}
-	if (ARRAY_SIZE(ref_kind) <= i)
-		return 0;
-
-	/* Don't add types the caller doesn't want */
-	if ((kind & ref_list->kinds) == 0)
-		return 0;
-
-	if (!match_patterns(cb->pattern, refname))
-		return 0;
-
-	commit = NULL;
-	if (ref_list->verbose || ref_list->with_commit || merge_filter != NO_FILTER) {
-		commit = lookup_commit_reference_gently(oid->hash, 1);
-		if (!commit) {
-			cb->ret = error(_("branch '%s' does not point at a commit"), refname);
-			return 0;
-		}
-
-		/* Filter with with_commit if specified */
-		if (!is_descendant_of(commit, ref_list->with_commit))
-			return 0;
-
-		if (merge_filter != NO_FILTER)
-			add_pending_object(&ref_list->revs,
-					   (struct object *)commit, refname);
-	}
-
-	ALLOC_GROW(ref_list->list, ref_list->index + 1, ref_list->alloc);
-
-	/* Record the new item */
-	newitem = &(ref_list->list[ref_list->index++]);
-	newitem->name = xstrdup(refname);
-	newitem->kind = kind;
-	newitem->commit = commit;
-	newitem->width = utf8_strwidth(refname);
-	newitem->dest = resolve_symref(orig_refname, prefix);
-	newitem->ignore = 0;
-	/* adjust for "remotes/" */
-	if (newitem->kind == REF_REMOTE_BRANCH &&
-	    ref_list->kinds != REF_REMOTE_BRANCH)
-		newitem->width += 8;
-	if (newitem->width > ref_list->maxwidth)
-		ref_list->maxwidth = newitem->width;
-
-	return 0;
-}
-
-static void free_ref_list(struct ref_list *ref_list)
-{
-	int i;
-
-	for (i = 0; i < ref_list->index; i++) {
-		free(ref_list->list[i].name);
-		free(ref_list->list[i].dest);
-	}
-	free(ref_list->list);
-}
-
-static int ref_cmp(const void *r1, const void *r2)
-{
-	struct ref_item *c1 = (struct ref_item *)(r1);
-	struct ref_item *c2 = (struct ref_item *)(r2);
-
-	if (c1->kind != c2->kind)
-		return c1->kind - c2->kind;
-	return strcmp(c1->name, c2->name);
-}
-
 static void fill_tracking_info(struct strbuf *stat, const char *branch_name,
 		int show_upstream_ref)
 {
 	int ours, theirs;
 	char *ref = NULL;
 	struct branch *branch = branch_get(branch_name);
