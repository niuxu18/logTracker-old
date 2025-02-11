 	}
 	*cp = '/';
 
 	return cp + 1;
 }
 
-/* Call this if the destination dir (which is assumed to be in curr_dir)
- * does not yet exist and we can't create it due to being in dry-run
- * mode.  We'll fix dirs that can be relative to the non-existent dir. */
-static void fix_basis_dirs(void)
+/* This function checks on our alternate-basis directories.  If we're in
+ * dry-run mode and the destination dir does not yet exist, we'll try to
+ * tweak any dest-relative paths to make them work for a dry-run (the
+ * destination dir must be in curr_dir[] when this function is called).
+ * We also warn about any arg that is non-existent or not a directory. */
+static void check_alt_basis_dirs(void)
 {
-	char **dir, *new, *slash;
-	int len;
-
-	if (dry_run <= 1)
-		return;
-
-	slash = strrchr(curr_dir, '/');
+	STRUCT_STAT st;
+	char **dir_p, *slash = strrchr(curr_dir, '/');
 
-	for (dir = basis_dir; *dir; dir++) {
-		if (**dir == '/')
-			continue;
-		len = curr_dir_len + 1 + strlen(*dir) + 1;
-		if (!(new = new_array(char, len)))
-			out_of_memory("fix_basis_dirs");
-		if (slash && strncmp(*dir, "../", 3) == 0) {
-		    /* We want to remove only one leading "../" prefix for
-		     * the directory we couldn't create in dry-run mode:
-		     * this ensures that any other ".." references get
-		     * evaluated the same as they would for a live copy. */
-		    *slash = '\0';
-		    pathjoin(new, len, curr_dir, *dir + 3);
-		    *slash = '/';
-		} else
-		    pathjoin(new, len, curr_dir, *dir);
-		*dir = new;
+	for (dir_p = basis_dir; *dir_p; dir_p++) {
+		if (dry_run > 1 && **dir_p != '/') {
+			int len = curr_dir_len + 1 + strlen(*dir_p) + 1;
+			char *new = new_array(char, len);
+			if (!new)
+				out_of_memory("check_alt_basis_dirs");
+			if (slash && strncmp(*dir_p, "../", 3) == 0) {
+			    /* We want to remove only one leading "../" prefix for
+			     * the directory we couldn't create in dry-run mode:
+			     * this ensures that any other ".." references get
+			     * evaluated the same as they would for a live copy. */
+			    *slash = '\0';
+			    pathjoin(new, len, curr_dir, *dir_p + 3);
+			    *slash = '/';
+			} else
+			    pathjoin(new, len, curr_dir, *dir_p);
+			*dir_p = new;
+		}
+		if (do_stat(*dir_p, &st) < 0) {
+			rprintf(FWARNING, "%s arg does not exist: %s\n",
+				dest_option, *dir_p);
+		} else if (!S_ISDIR(st.st_mode)) {
+			rprintf(FWARNING, "%s arg is not a dir: %s\n",
+				dest_option, *dir_p);
+		}
 	}
 }
 
 /* This is only called by the sender. */
-static void read_final_goodbye(int f_in, int f_out)
+static void read_final_goodbye(int f_in)
 {
-	int i;
+	int i, iflags, xlen;
+	uchar fnamecmp_type;
+	char xname[MAXPATHLEN];
 
 	if (protocol_version < 29)
 		i = read_int(f_in);
 	else {
-		while ((i = read_int(f_in)) == the_file_list->count
-		    && read_shortint(f_in) == ITEM_IS_NEW) {
-			/* Forward the keep-alive (no-op) to the receiver. */
-			write_int(f_out, the_file_list->count);
-			write_shortint(f_out, ITEM_IS_NEW);
-		}
+		i = read_ndx_and_attrs(f_in, &iflags, &fnamecmp_type,
+				       xname, &xlen);
 	}
 
-	if (i != -1) {
+	if (i != NDX_DONE) {
 		rprintf(FERROR, "Invalid packet at end of run (%d) [%s]\n",
 			i, who_am_i());
 		exit_cleanup(RERR_PROTOCOL);
 	}
 }
 
