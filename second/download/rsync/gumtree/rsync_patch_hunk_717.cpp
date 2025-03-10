 	   more useful */
 	if (list_only && !recurse)
 		argstr[x++] = 'r';
 
 	argstr[x] = 0;
 
-	if (x != 1) args[ac++] = argstr;
+	if (x != 1)
+		args[ac++] = argstr;
 
 	if (block_size) {
-		snprintf(bsize,sizeof(bsize),"-B%d",block_size);
-		args[ac++] = bsize;
+		if (asprintf(&arg, "-B%u", block_size) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
 	if (max_delete && am_sender) {
-		snprintf(mdelete,sizeof(mdelete),"--max-delete=%d",max_delete);
-		args[ac++] = mdelete;
+		if (asprintf(&arg, "--max-delete=%d", max_delete) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
-	if (batch_prefix != NULL) {
-		char *fmt = "";
-		if (write_batch)
-			fmt = "--write-batch=%s";
-		else
-		if (read_batch)
-			fmt = "--read-batch=%s";
-		snprintf(fext,sizeof(fext),fmt,batch_prefix);
-		args[ac++] = fext;
+	if (batch_prefix) {
+		char *r_or_w = write_batch ? "write" : "read";
+		if (asprintf(&arg, "--%s-batch=%s", r_or_w, batch_prefix) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
 	if (io_timeout) {
-		snprintf(iotime,sizeof(iotime),"--timeout=%d",io_timeout);
-		args[ac++] = iotime;
+		if (asprintf(&arg, "--timeout=%d", io_timeout) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
 	if (bwlimit) {
-		snprintf(bw,sizeof(bw),"--bwlimit=%d",bwlimit);
-		args[ac++] = bw;
+		if (asprintf(&arg, "--bwlimit=%d", bwlimit) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
 	if (backup_dir) {
 		args[ac++] = "--backup-dir";
 		args[ac++] = backup_dir;
 	}
 
 	/* Only send --suffix if it specifies a non-default value. */
-	if (strcmp(backup_suffix, backup_dir? "" : BACKUP_SUFFIX) != 0) {
-		char *s = new_array(char, 9+backup_suffix_len+1);
-		if (!s)
-			out_of_memory("server_options");
+	if (strcmp(backup_suffix, backup_dir ? "" : BACKUP_SUFFIX) != 0) {
 		/* We use the following syntax to avoid weirdness with '~'. */
-		sprintf(s, "--suffix=%s", backup_suffix);
-		args[ac++] = s;
+		if (asprintf(&arg, "--suffix=%s", backup_suffix) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
-	if (delete_mode && !delete_excluded)
-		args[ac++] = "--delete";
-
 	if (delete_excluded)
 		args[ac++] = "--delete-excluded";
+	else if (delete_mode)
+		args[ac++] = "--delete";
 
 	if (size_only)
 		args[ac++] = "--size-only";
 
 	if (modify_window_set) {
-		snprintf(mwindow,sizeof(mwindow),"--modify-window=%d",
-			 modify_window);
-		args[ac++] = mwindow;
+		if (asprintf(&arg, "--modify-window=%d", modify_window) < 0)
+			goto oom;
+		args[ac++] = arg;
 	}
 
 	if (keep_partial)
 		args[ac++] = "--partial";
 
 	if (force_delete)
