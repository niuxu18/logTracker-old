 		struct ref_entry *entry = dir->entries[i];
 		if (entry->flag & REF_DIR)
 			prime_ref_dir(get_ref_dir(entry));
 	}
 }
 
-static int entry_matches(struct ref_entry *entry, const struct string_list *list)
-{
-	return list && string_list_has_string(list, entry->name);
-}
-
 struct nonmatching_ref_data {
 	const struct string_list *skip;
-	struct ref_entry *found;
+	const char *conflicting_refname;
 };
 
 static int nonmatching_ref_fn(struct ref_entry *entry, void *vdata)
 {
 	struct nonmatching_ref_data *data = vdata;
 
-	if (entry_matches(entry, data->skip))
+	if (data->skip && string_list_has_string(data->skip, entry->name))
 		return 0;
 
-	data->found = entry;
+	data->conflicting_refname = entry->name;
 	return 1;
 }
 
-static void report_refname_conflict(struct ref_entry *entry,
-				    const char *refname)
-{
-	error("'%s' exists; cannot create '%s'", entry->name, refname);
-}
-
 /*
- * Return true iff a reference named refname could be created without
- * conflicting with the name of an existing reference in dir.  If
- * skip is non-NULL, ignore potential conflicts with refs in skip
- * (e.g., because they are scheduled for deletion in the same
- * operation).
+ * Return 0 if a reference named refname could be created without
+ * conflicting with the name of an existing reference in dir.
+ * Otherwise, return a negative value and write an explanation to err.
+ * If extras is non-NULL, it is a list of additional refnames with
+ * which refname is not allowed to conflict. If skip is non-NULL,
+ * ignore potential conflicts with refs in skip (e.g., because they
+ * are scheduled for deletion in the same operation). Behavior is
+ * undefined if the same name is listed in both extras and skip.
  *
  * Two reference names conflict if one of them exactly matches the
- * leading components of the other; e.g., "foo/bar" conflicts with
- * both "foo" and with "foo/bar/baz" but not with "foo/bar" or
- * "foo/barbados".
- *
- * skip must be sorted.
- */
-static int is_refname_available(const char *refname,
-				const struct string_list *skip,
-				struct ref_dir *dir)
+ * leading components of the other; e.g., "refs/foo/bar" conflicts
+ * with both "refs/foo" and with "refs/foo/bar/baz" but not with
+ * "refs/foo/bar" or "refs/foo/barbados".
+ *
+ * extras and skip must be sorted.
+ */
+static int verify_refname_available(const char *refname,
+				    const struct string_list *extras,
+				    const struct string_list *skip,
+				    struct ref_dir *dir,
+				    struct strbuf *err)
 {
 	const char *slash;
-	size_t len;
 	int pos;
-	char *dirname;
+	struct strbuf dirname = STRBUF_INIT;
+	int ret = -1;
+
+	/*
+	 * For the sake of comments in this function, suppose that
+	 * refname is "refs/foo/bar".
+	 */
 
+	assert(err);
+
+	strbuf_grow(&dirname, strlen(refname) + 1);
 	for (slash = strchr(refname, '/'); slash; slash = strchr(slash + 1, '/')) {
+		/* Expand dirname to the new prefix, not including the trailing slash: */
+		strbuf_add(&dirname, refname + dirname.len, slash - refname - dirname.len);
+
 		/*
-		 * We are still at a leading dir of the refname; we are
-		 * looking for a conflict with a leaf entry.
-		 *
-		 * If we find one, we still must make sure it is
-		 * not in "skip".
+		 * We are still at a leading dir of the refname (e.g.,
+		 * "refs/foo"; if there is a reference with that name,
+		 * it is a conflict, *unless* it is in skip.
 		 */
-		pos = search_ref_dir(dir, refname, slash - refname);
-		if (pos >= 0) {
-			struct ref_entry *entry = dir->entries[pos];
-			if (entry_matches(entry, skip))
-				return 1;
-			report_refname_conflict(entry, refname);
-			return 0;
+		if (dir) {
+			pos = search_ref_dir(dir, dirname.buf, dirname.len);
+			if (pos >= 0 &&
+			    (!skip || !string_list_has_string(skip, dirname.buf))) {
+				/*
+				 * We found a reference whose name is
+				 * a proper prefix of refname; e.g.,
+				 * "refs/foo", and is not in skip.
+				 */
+				strbuf_addf(err, "'%s' exists; cannot create '%s'",
+					    dirname.buf, refname);
+				goto cleanup;
+			}
 		}
 
+		if (extras && string_list_has_string(extras, dirname.buf) &&
+		    (!skip || !string_list_has_string(skip, dirname.buf))) {
+			strbuf_addf(err, "cannot process '%s' and '%s' at the same time",
+				    refname, dirname.buf);
+			goto cleanup;
+		}
 
 		/*
 		 * Otherwise, we can try to continue our search with
-		 * the next component; if we come up empty, we know
-		 * there is nothing under this whole prefix.
+		 * the next component. So try to look up the
+		 * directory, e.g., "refs/foo/". If we come up empty,
+		 * we know there is nothing under this whole prefix,
+		 * but even in that case we still have to continue the
+		 * search for conflicts with extras.
 		 */
-		pos = search_ref_dir(dir, refname, slash + 1 - refname);
-		if (pos < 0)
-			return 1;
-
-		dir = get_ref_dir(dir->entries[pos]);
+		strbuf_addch(&dirname, '/');
+		if (dir) {
+			pos = search_ref_dir(dir, dirname.buf, dirname.len);
+			if (pos < 0) {
+				/*
+				 * There was no directory "refs/foo/",
+				 * so there is nothing under this
+				 * whole prefix. So there is no need
+				 * to continue looking for conflicting
+				 * references. But we need to continue
+				 * looking for conflicting extras.
+				 */
+				dir = NULL;
+			} else {
+				dir = get_ref_dir(dir->entries[pos]);
+			}
+		}
 	}
 
 	/*
-	 * We are at the leaf of our refname; we want to
-	 * make sure there are no directories which match it.
+	 * We are at the leaf of our refname (e.g., "refs/foo/bar").
+	 * There is no point in searching for a reference with that
+	 * name, because a refname isn't considered to conflict with
+	 * itself. But we still need to check for references whose
+	 * names are in the "refs/foo/bar/" namespace, because they
+	 * *do* conflict.
 	 */
-	len = strlen(refname);
-	dirname = xmallocz(len + 1);
-	sprintf(dirname, "%s/", refname);
-	pos = search_ref_dir(dir, dirname, len + 1);
-	free(dirname);
+	strbuf_addstr(&dirname, refname + dirname.len);
+	strbuf_addch(&dirname, '/');
+
+	if (dir) {
+		pos = search_ref_dir(dir, dirname.buf, dirname.len);
+
+		if (pos >= 0) {
+			/*
+			 * We found a directory named "$refname/"
+			 * (e.g., "refs/foo/bar/"). It is a problem
+			 * iff it contains any ref that is not in
+			 * "skip".
+			 */
+			struct nonmatching_ref_data data;
+
+			data.skip = skip;
+			data.conflicting_refname = NULL;
+			dir = get_ref_dir(dir->entries[pos]);
+			sort_ref_dir(dir);
+			if (do_for_each_entry_in_dir(dir, 0, nonmatching_ref_fn, &data)) {
+				strbuf_addf(err, "'%s' exists; cannot create '%s'",
+					    data.conflicting_refname, refname);
+				goto cleanup;
+			}
+		}
+	}
 
-	if (pos >= 0) {
+	if (extras) {
 		/*
-		 * We found a directory named "refname". It is a
-		 * problem iff it contains any ref that is not
-		 * in "skip".
+		 * Check for entries in extras that start with
+		 * "$refname/". We do that by looking for the place
+		 * where "$refname/" would be inserted in extras. If
+		 * there is an entry at that position that starts with
+		 * "$refname/" and is not in skip, then we have a
+		 * conflict.
 		 */
-		struct ref_entry *entry = dir->entries[pos];
-		struct ref_dir *dir = get_ref_dir(entry);
-		struct nonmatching_ref_data data;
+		for (pos = string_list_find_insert_index(extras, dirname.buf, 0);
+		     pos < extras->nr; pos++) {
+			const char *extra_refname = extras->items[pos].string;
 
-		data.skip = skip;
-		sort_ref_dir(dir);
-		if (!do_for_each_entry_in_dir(dir, 0, nonmatching_ref_fn, &data))
-			return 1;
+			if (!starts_with(extra_refname, dirname.buf))
+				break;
 
-		report_refname_conflict(data.found, refname);
-		return 0;
+			if (!skip || !string_list_has_string(skip, extra_refname)) {
+				strbuf_addf(err, "cannot process '%s' and '%s' at the same time",
+					    refname, extra_refname);
+				goto cleanup;
+			}
+		}
 	}
 
-	/*
-	 * There is no point in searching for another leaf
-	 * node which matches it; such an entry would be the
-	 * ref we are looking for, not a conflict.
-	 */
-	return 1;
+	/* No conflicts were found */
+	ret = 0;
+
+cleanup:
+	strbuf_release(&dirname);
+	return ret;
 }
 
 struct packed_ref_cache {
 	struct ref_entry *root;
 
 	/*
