 int is_directory(const char *path)
 {
 	struct stat st;
 	return (!stat(path, &st) && S_ISDIR(st.st_mode));
 }
 
+/* removes the last path component from 'path' except if 'path' is root */
+static void strip_last_component(struct strbuf *path)
+{
+	size_t offset = offset_1st_component(path->buf);
+	size_t len = path->len;
+
+	/* Find start of the last component */
+	while (offset < len && !is_dir_sep(path->buf[len - 1]))
+		len--;
+	/* Skip sequences of multiple path-separators */
+	while (offset < len && is_dir_sep(path->buf[len - 1]))
+		len--;
+
+	strbuf_setlen(path, len);
+}
+
+/* get (and remove) the next component in 'remaining' and place it in 'next' */
+static void get_next_component(struct strbuf *next, struct strbuf *remaining)
+{
+	char *start = NULL;
+	char *end = NULL;
+
+	strbuf_reset(next);
+
+	/* look for the next component */
+	/* Skip sequences of multiple path-separators */
+	for (start = remaining->buf; is_dir_sep(*start); start++)
+		; /* nothing */
+	/* Find end of the path component */
+	for (end = start; *end && !is_dir_sep(*end); end++)
+		; /* nothing */
+
+	strbuf_add(next, start, end - start);
+	/* remove the component from 'remaining' */
+	strbuf_remove(remaining, 0, end - remaining->buf);
+}
+
+/* copies root part from remaining to resolved, canonicalizing it on the way */
+static void get_root_part(struct strbuf *resolved, struct strbuf *remaining)
+{
+	int offset = offset_1st_component(remaining->buf);
+
+	strbuf_reset(resolved);
+	strbuf_add(resolved, remaining->buf, offset);
+#ifdef GIT_WINDOWS_NATIVE
+	convert_slashes(resolved->buf);
+#endif
+	strbuf_remove(remaining, 0, offset);
+}
+
 /* We allow "recursive" symbolic links. Only within reason, though. */
-#define MAXDEPTH 5
+#ifndef MAXSYMLINKS
+#define MAXSYMLINKS 32
+#endif
 
 /*
  * Return the real path (i.e., absolute path, with symlinks resolved
  * and extra slashes removed) equivalent to the specified path.  (If
  * you want an absolute path but don't mind links, use
- * absolute_path().)  The return value is a pointer to a static
- * buffer.
+ * absolute_path().)  Places the resolved realpath in the provided strbuf.
  *
- * The input and all intermediate paths must be shorter than MAX_PATH.
  * The directory part of path (i.e., everything up to the last
  * dir_sep) must denote a valid, existing directory, but the last
  * component need not exist.  If die_on_error is set, then die with an
  * informative error message if there is a problem.  Otherwise, return
  * NULL on errors (without generating any output).
- *
- * If path is our buffer, then return path, as it's already what the
- * user wants.
  */
-static const char *real_path_internal(const char *path, int die_on_error)
+char *strbuf_realpath(struct strbuf *resolved, const char *path,
+		      int die_on_error)
 {
-	static struct strbuf sb = STRBUF_INIT;
+	struct strbuf remaining = STRBUF_INIT;
+	struct strbuf next = STRBUF_INIT;
+	struct strbuf symlink = STRBUF_INIT;
 	char *retval = NULL;
-
-	/*
-	 * If we have to temporarily chdir(), store the original CWD
-	 * here so that we can chdir() back to it at the end of the
-	 * function:
-	 */
-	struct strbuf cwd = STRBUF_INIT;
-
-	int depth = MAXDEPTH;
-	char *last_elem = NULL;
+	int num_symlinks = 0;
 	struct stat st;
 
-	/* We've already done it */
-	if (path == sb.buf)
-		return path;
-
 	if (!*path) {
 		if (die_on_error)
 			die("The empty string is not a valid path");
 		else
 			goto error_out;
 	}
 
-	strbuf_reset(&sb);
-	strbuf_addstr(&sb, path);
+	strbuf_addstr(&remaining, path);
+	get_root_part(resolved, &remaining);
 
-	while (depth--) {
-		if (!is_directory(sb.buf)) {
-			char *last_slash = find_last_dir_sep(sb.buf);
-			if (last_slash) {
-				last_elem = xstrdup(last_slash + 1);
-				strbuf_setlen(&sb, last_slash - sb.buf + 1);
-			} else {
-				last_elem = xmemdupz(sb.buf, sb.len);
-				strbuf_reset(&sb);
-			}
+	if (!resolved->len) {
+		/* relative path; can use CWD as the initial resolved path */
+		if (strbuf_getcwd(resolved)) {
+			if (die_on_error)
+				die_errno("unable to get current working directory");
+			else
+				goto error_out;
 		}
+	}
 
-		if (sb.len) {
-			if (!cwd.len && strbuf_getcwd(&cwd)) {
+	/* Iterate over the remaining path components */
+	while (remaining.len > 0) {
+		get_next_component(&next, &remaining);
+
+		if (next.len == 0) {
+			continue; /* empty component */
+		} else if (next.len == 1 && !strcmp(next.buf, ".")) {
+			continue; /* '.' component */
+		} else if (next.len == 2 && !strcmp(next.buf, "..")) {
+			/* '..' component; strip the last path component */
+			strip_last_component(resolved);
+			continue;
+		}
+
+		/* append the next component and resolve resultant path */
+		if (!is_dir_sep(resolved->buf[resolved->len - 1]))
+			strbuf_addch(resolved, '/');
+		strbuf_addbuf(resolved, &next);
+
+		if (lstat(resolved->buf, &st)) {
+			/* error out unless this was the last component */
+			if (errno != ENOENT || remaining.len) {
 				if (die_on_error)
-					die_errno("Could not get current working directory");
+					die_errno("Invalid path '%s'",
+						  resolved->buf);
 				else
 					goto error_out;
 			}
+		} else if (S_ISLNK(st.st_mode)) {
+			ssize_t len;
+			strbuf_reset(&symlink);
+
+			if (num_symlinks++ > MAXSYMLINKS) {
+				errno = ELOOP;
 
-			if (chdir(sb.buf)) {
 				if (die_on_error)
-					die_errno("Could not switch to '%s'",
-						  sb.buf);
+					die("More than %d nested symlinks "
+					    "on path '%s'", MAXSYMLINKS, path);
 				else
 					goto error_out;
 			}
-		}
-		if (strbuf_getcwd(&sb)) {
-			if (die_on_error)
-				die_errno("Could not get current working directory");
-			else
-				goto error_out;
-		}
-
-		if (last_elem) {
-			if (sb.len && !is_dir_sep(sb.buf[sb.len - 1]))
-				strbuf_addch(&sb, '/');
-			strbuf_addstr(&sb, last_elem);
-			free(last_elem);
-			last_elem = NULL;
-		}
 
-		if (!lstat(sb.buf, &st) && S_ISLNK(st.st_mode)) {
-			struct strbuf next_sb = STRBUF_INIT;
-			ssize_t len = strbuf_readlink(&next_sb, sb.buf, 0);
+			len = strbuf_readlink(&symlink, resolved->buf,
+					      st.st_size);
 			if (len < 0) {
 				if (die_on_error)
 					die_errno("Invalid symlink '%s'",
-						  sb.buf);
+						  resolved->buf);
 				else
 					goto error_out;
 			}
-			strbuf_swap(&sb, &next_sb);
-			strbuf_release(&next_sb);
-		} else
-			break;
+
+			if (is_absolute_path(symlink.buf)) {
+				/* absolute symlink; set resolved to root */
+				get_root_part(resolved, &symlink);
+			} else {
+				/*
+				 * relative symlink
+				 * strip off the last component since it will
+				 * be replaced with the contents of the symlink
+				 */
+				strip_last_component(resolved);
+			}
+
+			/*
+			 * if there are still remaining components to resolve
+			 * then append them to symlink
+			 */
+			if (remaining.len) {
+				strbuf_addch(&symlink, '/');
+				strbuf_addbuf(&symlink, &remaining);
+			}
+
+			/*
+			 * use the symlink as the remaining components that
+			 * need to be resloved
+			 */
+			strbuf_swap(&symlink, &remaining);
+		}
 	}
 
-	retval = sb.buf;
+	retval = resolved->buf;
+
 error_out:
-	free(last_elem);
-	if (cwd.len && chdir(cwd.buf))
-		die_errno("Could not change back to '%s'", cwd.buf);
-	strbuf_release(&cwd);
+	strbuf_release(&remaining);
+	strbuf_release(&next);
+	strbuf_release(&symlink);
+
+	if (!retval)
+		strbuf_reset(resolved);
 
 	return retval;
 }
 
 const char *real_path(const char *path)
 {
-	return real_path_internal(path, 1);
+	static struct strbuf realpath = STRBUF_INIT;
+	return strbuf_realpath(&realpath, path, 1);
 }
 
 const char *real_path_if_valid(const char *path)
 {
-	return real_path_internal(path, 0);
+	static struct strbuf realpath = STRBUF_INIT;
+	return strbuf_realpath(&realpath, path, 0);
+}
+
+char *real_pathdup(const char *path)
+{
+	struct strbuf realpath = STRBUF_INIT;
+	char *retval = NULL;
+
+	if (strbuf_realpath(&realpath, path, 0))
+		retval = strbuf_detach(&realpath, NULL);
+
+	strbuf_release(&realpath);
+
+	return retval;
 }
 
 /*
  * Use this to get an absolute path from a relative one. If you want
  * to resolve links, you should use real_path.
  */
