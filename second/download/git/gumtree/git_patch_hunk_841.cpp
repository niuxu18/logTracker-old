 	int res;
 	int errs = 0;
 	int read_stdin = 1;
 
 	for (i = 0; i < argc; i++) {
 		const char *arg = argv[i];
+		char *to_free = NULL;
 		int fd;
 
 		if (!strcmp(arg, "-")) {
 			res = apply_patch(state, 0, "<stdin>", options);
 			if (res < 0)
 				goto end;
 			errs |= res;
 			read_stdin = 0;
 			continue;
-		} else if (0 < state->prefix_length)
-			arg = prefix_filename(state->prefix,
-					      state->prefix_length,
-					      arg);
+		} else
+			arg = to_free = prefix_filename(state->prefix, arg);
 
 		fd = open(arg, O_RDONLY);
 		if (fd < 0) {
 			error(_("can't open patch '%s': %s"), arg, strerror(errno));
 			res = -128;
+			free(to_free);
 			goto end;
 		}
 		read_stdin = 0;
 		set_default_whitespace_mode(state);
 		res = apply_patch(state, fd, arg, options);
 		close(fd);
+		free(to_free);
 		if (res < 0)
 			goto end;
 		errs |= res;
 	}
 	set_default_whitespace_mode(state);
 	if (read_stdin) {
