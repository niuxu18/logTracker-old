 }
 
 /*
  * Free up any resources a file_struct has allocated
  * and clear the file.
  */
-void clear_file(struct file_struct *file, struct file_list *flist)
+void clear_file(struct file_struct *file)
 {
-	if (flist->hlink_pool && file->link_u.idev)
-		pool_free(flist->hlink_pool, 0, file->link_u.idev);
-	memset(file, 0, file_struct_len);
-	/* In an empty entry, dir.depth is an offset to the next non-empty
-	 * entry.  Likewise for length in the opposite direction.  We assume
+	/* The +1 zeros out the first char of the basename. */
+	memset(file, 0, FILE_STRUCT_LEN + 1);
+	/* In an empty entry, F_DEPTH() is an offset to the next non-empty
+	 * entry.  Likewise for len32 in the opposite direction.  We assume
 	 * that we're alone for now since flist_find() will adjust the counts
 	 * it runs into that aren't up-to-date. */
-	file->length = file->dir.depth = 1;
+	file->len32 = F_DEPTH(file) = 1;
 }
 
-/*
- * allocate a new file list
- */
-struct file_list *flist_new(int with_hlink, char *msg)
+/* Allocate a new file list. */
+struct file_list *flist_new(int flags, char *msg)
 {
 	struct file_list *flist;
 
-	flist = new(struct file_list);
-	if (!flist)
+	if (!(flist = new0(struct file_list)))
 		out_of_memory(msg);
 
-	memset(flist, 0, sizeof (struct file_list));
+	if (flags & FLIST_TEMP) {
+		if (!(flist->file_pool = pool_create(SMALL_EXTENT, 0,
+						out_of_memory, POOL_INTERN)))
+			out_of_memory(msg);
+	} else {
+		/* This is a doubly linked list with prev looping back to
+		 * the end of the list, but the last next pointer is NULL. */
+		if (!first_flist) {
+			flist->file_pool = pool_create(NORMAL_EXTENT, 0,
+						out_of_memory, POOL_INTERN);
+			if (!flist->file_pool)
+				out_of_memory(msg);
 
-	if (!(flist->file_pool = pool_create(FILE_EXTENT, 0,
-	    out_of_memory, POOL_INTERN)))
-		out_of_memory(msg);
+			flist->ndx_start = flist->flist_num = inc_recurse ? 1 : 0;
 
-#ifdef SUPPORT_HARD_LINKS
-	if (with_hlink && preserve_hard_links) {
-		if (!(flist->hlink_pool = pool_create(HLINK_EXTENT,
-		    sizeof (struct idev), out_of_memory, POOL_INTERN)))
-			out_of_memory(msg);
+			first_flist = cur_flist = flist->prev = flist;
+		} else {
+			struct file_list *prev = first_flist->prev;
+
+			flist->file_pool = first_flist->file_pool;
+
+			flist->ndx_start = prev->ndx_start + prev->used + 1;
+			flist->flist_num = prev->flist_num + 1;
+
+			flist->prev = prev;
+			prev->next = first_flist->prev = flist;
+		}
+		flist->pool_boundary = pool_boundary(flist->file_pool, 0);
+		flist_cnt++;
 	}
-#endif
 
 	return flist;
 }
 
-/*
- * free up all elements in a flist
- */
+/* Free up all elements in a flist. */
 void flist_free(struct file_list *flist)
 {
-	pool_destroy(flist->file_pool);
-	pool_destroy(flist->hlink_pool);
+	if (!flist->prev) {
+		/* Was FLIST_TEMP dir-list. */
+	} else if (flist == flist->prev) {
+		first_flist = cur_flist = NULL;
+		file_total = 0;
+		flist_cnt = 0;
+	} else {
+		if (flist == cur_flist)
+			cur_flist = flist->next;
+		if (flist == first_flist)
+			first_flist = first_flist->next;
+		else {
+			flist->prev->next = flist->next;
+			if (!flist->next)
+				flist->next = first_flist;
+		}
+		flist->next->prev = flist->prev;
+		file_total -= flist->used;
+		flist_cnt--;
+	}
+
+	if (!flist->prev || !flist_cnt)
+		pool_destroy(flist->file_pool);
+	else
+		pool_free_old(flist->file_pool, flist->pool_boundary);
+
+	if (flist->sorted && flist->sorted != flist->files)
+		free(flist->sorted);
 	free(flist->files);
 	free(flist);
 }
 
-/*
- * This routine ensures we don't have any duplicate names in our file list.
- * duplicate names can cause corruption because of the pipelining
- */
-static void clean_flist(struct file_list *flist, int strip_root, int no_dups)
+/* This routine ensures we don't have any duplicate names in our file list.
+ * duplicate names can cause corruption because of the pipelining. */
+static void flist_sort_and_clean(struct file_list *flist, int strip_root)
 {
 	char fbuf[MAXPATHLEN];
-	int i, prev_i = 0;
+	int i, prev_i;
 
 	if (!flist)
 		return;
-	if (flist->count == 0) {
+	if (flist->used == 0) {
 		flist->high = -1;
+		flist->low = 0;
 		return;
 	}
 
-	qsort(flist->files, flist->count,
-	    sizeof flist->files[0], (int (*)())file_compare);
+	fsort(flist->sorted, flist->used);
 
-	for (i = no_dups? 0 : flist->count; i < flist->count; i++) {
-		if (flist->files[i]->basename) {
-			prev_i = i;
-			break;
+	if (!am_sender || inc_recurse) {
+		for (i = prev_i = 0; i < flist->used; i++) {
+			if (F_IS_ACTIVE(flist->sorted[i])) {
+				prev_i = i;
+				break;
+			}
 		}
+		flist->low = prev_i;
+	} else {
+		i = prev_i = flist->used - 1;
+		flist->low = 0;
 	}
-	flist->low = prev_i;
-	while (++i < flist->count) {
+
+	while (++i < flist->used) {
 		int j;
-		struct file_struct *file = flist->files[i];
+		struct file_struct *file = flist->sorted[i];
 
-		if (!file->basename)
+		if (!F_IS_ACTIVE(file))
 			continue;
-		if (f_name_cmp(file, flist->files[prev_i]) == 0)
+		if (f_name_cmp(file, flist->sorted[prev_i]) == 0)
 			j = prev_i;
 		else if (protocol_version >= 29 && S_ISDIR(file->mode)) {
 			int save_mode = file->mode;
 			/* Make sure that this directory doesn't duplicate a
 			 * non-directory earlier in the list. */
 			flist->high = prev_i;
 			file->mode = S_IFREG;
 			j = flist_find(flist, file);
 			file->mode = save_mode;
 		} else
 			j = -1;
 		if (j >= 0) {
-			struct file_struct *fp = flist->files[j];
 			int keep, drop;
 			/* If one is a dir and the other is not, we want to
 			 * keep the dir because it might have contents in the
-			 * list. */
-			if (S_ISDIR(file->mode) != S_ISDIR(fp->mode)) {
-				if (S_ISDIR(file->mode))
+			 * list.  Otherwise keep the first one. */
+			if (S_ISDIR(file->mode)) {
+				struct file_struct *fp = flist->sorted[j];
+				if (!S_ISDIR(fp->mode))
 					keep = i, drop = j;
-				else
+				else {
+					if (am_sender)
+						file->flags |= FLAG_DUPLICATE;
+					else { /* Make sure we merge our vital flags. */
+						fp->flags |= file->flags & (FLAG_TOP_DIR|FLAG_CONTENT_DIR);
+						fp->flags &= file->flags | ~FLAG_IMPLIED_DIR;
+					}
 					keep = j, drop = i;
+				}
 			} else
 				keep = j, drop = i;
-			if (verbose > 1 && !am_server) {
-				rprintf(FINFO,
-					"removing duplicate name %s from file list (%d)\n",
-					f_name(file, fbuf), drop);
-			}
-			/* Make sure we don't lose track of a user-specified
-			 * top directory. */
-			flist->files[keep]->flags |= flist->files[drop]->flags
-						   & (FLAG_TOP_DIR|FLAG_DEL_HERE);
 
-			clear_file(flist->files[drop], flist);
+			if (!am_sender) {
+				if (verbose > 1) {
+					rprintf(FINFO,
+					    "removing duplicate name %s from file list (%d)\n",
+					    f_name(file, fbuf), drop + flist->ndx_start);
+				}
+				clear_file(flist->sorted[drop]);
+			}
 
 			if (keep == i) {
 				if (flist->low == drop) {
 					for (j = drop + 1;
-					     j < i && !flist->files[j]->basename;
+					     j < i && !F_IS_ACTIVE(flist->sorted[j]);
 					     j++) {}
 					flist->low = j;
 				}
 				prev_i = i;
 			}
 		} else
 			prev_i = i;
 	}
-	flist->high = no_dups ? prev_i : flist->count - 1;
+	flist->high = prev_i;
 
 	if (strip_root) {
 		/* We need to strip off the leading slashes for relative
 		 * paths, but this must be done _after_ the sorting phase. */
 		for (i = flist->low; i <= flist->high; i++) {
-			struct file_struct *file = flist->files[i];
+			struct file_struct *file = flist->sorted[i];
 
 			if (!file->dirname)
 				continue;
 			while (*file->dirname == '/')
 				file->dirname++;
 			if (!*file->dirname)
 				file->dirname = NULL;
 		}
 	}
 
-	if (prune_empty_dirs && no_dups) {
+	if (prune_empty_dirs && !am_sender) {
 		int j, prev_depth = 0;
 
 		prev_i = 0; /* It's OK that this isn't really true. */
 
 		for (i = flist->low; i <= flist->high; i++) {
-			struct file_struct *fp, *file = flist->files[i];
+			struct file_struct *fp, *file = flist->sorted[i];
 
-			/* This temporarily abuses the dir.depth value for a
+			/* This temporarily abuses the F_DEPTH() value for a
 			 * directory that is in a chain that might get pruned.
 			 * We restore the old value if it gets a reprieve. */
-			if (S_ISDIR(file->mode) && file->dir.depth) {
+			if (S_ISDIR(file->mode) && F_DEPTH(file)) {
 				/* Dump empty dirs when coming back down. */
-				for (j = prev_depth; j >= file->dir.depth; j--) {
-					fp = flist->files[prev_i];
-					if (fp->dir.depth >= 0)
+				for (j = prev_depth; j >= F_DEPTH(file); j--) {
+					fp = flist->sorted[prev_i];
+					if (F_DEPTH(fp) >= 0)
 						break;
-					prev_i = -fp->dir.depth-1;
-					clear_file(fp, flist);
+					prev_i = -F_DEPTH(fp)-1;
+					clear_file(fp);
 				}
-				prev_depth = file->dir.depth;
+				prev_depth = F_DEPTH(file);
 				if (is_excluded(f_name(file, fbuf), 1,
 						       ALL_FILTERS)) {
 					/* Keep dirs through this dir. */
 					for (j = prev_depth-1; ; j--) {
-						fp = flist->files[prev_i];
-						if (fp->dir.depth >= 0)
+						fp = flist->sorted[prev_i];
+						if (F_DEPTH(fp) >= 0)
 							break;
-						prev_i = -fp->dir.depth-1;
-						fp->dir.depth = j;
+						prev_i = -F_DEPTH(fp)-1;
+						F_DEPTH(fp) = j;
 					}
 				} else
-					file->dir.depth = -prev_i-1;
+					F_DEPTH(file) = -prev_i-1;
 				prev_i = i;
 			} else {
 				/* Keep dirs through this non-dir. */
 				for (j = prev_depth; ; j--) {
-					fp = flist->files[prev_i];
-					if (fp->dir.depth >= 0)
+					fp = flist->sorted[prev_i];
+					if (F_DEPTH(fp) >= 0)
 						break;
-					prev_i = -fp->dir.depth-1;
-					fp->dir.depth = j;
+					prev_i = -F_DEPTH(fp)-1;
+					F_DEPTH(fp) = j;
 				}
 			}
 		}
-		/* Dump empty all remaining empty dirs. */
+		/* Dump all remaining empty dirs. */
 		while (1) {
-			struct file_struct *fp = flist->files[prev_i];
-			if (fp->dir.depth >= 0)
+			struct file_struct *fp = flist->sorted[prev_i];
+			if (F_DEPTH(fp) >= 0)
 				break;
-			prev_i = -fp->dir.depth-1;
-			clear_file(fp, flist);
+			prev_i = -F_DEPTH(fp)-1;
+			clear_file(fp);
 		}
 
 		for (i = flist->low; i <= flist->high; i++) {
-			if (flist->files[i]->basename)
+			if (F_IS_ACTIVE(flist->sorted[i]))
 				break;
 		}
 		flist->low = i;
 		for (i = flist->high; i >= flist->low; i--) {
-			if (flist->files[i]->basename)
+			if (F_IS_ACTIVE(flist->sorted[i]))
 				break;
 		}
 		flist->high = i;
 	}
 }
 
 static void output_flist(struct file_list *flist)
 {
 	char uidbuf[16], gidbuf[16], depthbuf[16];
 	struct file_struct *file;
+	const char *root, *dir, *slash, *name, *trail;
 	const char *who = who_am_i();
 	int i;
 
-	for (i = 0; i < flist->count; i++) {
+	rprintf(FINFO, "[%s] flist start=%d, used=%d, low=%d, high=%d\n",
+		who, flist->ndx_start, flist->used, flist->low, flist->high);
+	for (i = 0; i < flist->used; i++) {
 		file = flist->files[i];
-		if ((am_root || am_sender) && preserve_uid)
-			snprintf(uidbuf, sizeof uidbuf, " uid=%ld", (long)file->uid);
-		else
+		if ((am_root || am_sender) && uid_ndx) {
+			snprintf(uidbuf, sizeof uidbuf, " uid=%u",
+				 F_OWNER(file));
+		} else
 			*uidbuf = '\0';
-		if (preserve_gid && file->gid != GID_NONE)
-			snprintf(gidbuf, sizeof gidbuf, " gid=%ld", (long)file->gid);
-		else
+		if (gid_ndx) {
+			static char parens[] = "(\0)\0\0\0";
+			char *pp = parens + (file->flags & FLAG_SKIP_GROUP ? 0 : 3);
+			snprintf(gidbuf, sizeof gidbuf, " gid=%s%u%s",
+				 pp, F_GROUP(file), pp + 2);
+		} else
 			*gidbuf = '\0';
 		if (!am_sender)
-			snprintf(depthbuf, sizeof depthbuf, "%d", file->dir.depth);
-		rprintf(FINFO, "[%s] i=%d %s %s%s%s%s mode=0%o len=%.0f%s%s flags=%x\n",
-			who, i, am_sender ? NS(file->dir.root) : depthbuf,
-			file->dirname ? file->dirname : "",
-			file->dirname ? "/" : "", NS(file->basename),
-			S_ISDIR(file->mode) ? "/" : "", (int)file->mode,
-			(double)file->length, uidbuf, gidbuf, file->flags);
+			snprintf(depthbuf, sizeof depthbuf, "%d", F_DEPTH(file));
+		if (F_IS_ACTIVE(file)) {
+			root = am_sender ? NS(F_PATHNAME(file)) : depthbuf;
+			if ((dir = file->dirname) == NULL)
+				dir = slash = "";
+			else
+				slash = "/";
+			name = file->basename;
+			trail = S_ISDIR(file->mode) ? "/" : "";
+		} else
+			root = dir = slash = name = trail = "";
+		rprintf(FINFO,
+			"[%s] i=%d %s %s%s%s%s mode=0%o len=%.0f%s%s flags=%x\n",
+			who, i + flist->ndx_start,
+			root, dir, slash, name, trail,
+			(int)file->mode, (double)F_LENGTH(file),
+			uidbuf, gidbuf, file->flags);
 	}
 }
 
 enum fnc_state { s_DIR, s_SLASH, s_BASE, s_TRAILING };
 enum fnc_type { t_PATH, t_ITEM };
 
+static int found_prefix;
+
 /* Compare the names of two file_struct entities, similar to how strcmp()
  * would do if it were operating on the joined strings.
  *
  * Some differences beginning with protocol_version 29: (1) directory names
  * are compared with an assumed trailing slash so that they compare in a
  * way that would cause them to sort immediately prior to any content they
