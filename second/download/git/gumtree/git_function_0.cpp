static int prepare_to_commit(const char *index_file, const char *prefix,
			     struct commit *current_head,
			     struct wt_status *s,
			     struct strbuf *author_ident)
{
	struct stat statbuf;
	struct strbuf committer_ident = STRBUF_INIT;
	int commitable;
	struct strbuf sb = STRBUF_INIT;
	const char *hook_arg1 = NULL;
	const char *hook_arg2 = NULL;
	int clean_message_contents = (cleanup_mode != CLEANUP_NONE);
	int old_display_comment_prefix;

	/* This checks and barfs if author is badly specified */
	determine_author_info(author_ident);

	if (!no_verify && run_commit_hook(use_editor, index_file, "pre-commit", NULL))
		return 0;

	if (squash_message) {
		/*
		 * Insert the proper subject line before other commit
		 * message options add their content.
		 */
		if (use_message && !strcmp(use_message, squash_message))
			strbuf_addstr(&sb, "squash! ");
		else {
			struct pretty_print_context ctx = {0};
			struct commit *c;
			c = lookup_commit_reference_by_name(squash_message);
			if (!c)
				die(_("could not lookup commit %s"), squash_message);
			ctx.output_encoding = get_commit_output_encoding();
			format_commit_message(c, "squash! %s\n\n", &sb,
					      &ctx);
		}
	}

	if (message.len) {
		strbuf_addbuf(&sb, &message);
		hook_arg1 = "message";
	} else if (logfile && !strcmp(logfile, "-")) {
		if (isatty(0))
			fprintf(stderr, _("(reading log message from standard input)\n"));
		if (strbuf_read(&sb, 0, 0) < 0)
			die_errno(_("could not read log from standard input"));
		hook_arg1 = "message";
	} else if (logfile) {
		if (strbuf_read_file(&sb, logfile, 0) < 0)
			die_errno(_("could not read log file '%s'"),
				  logfile);
		hook_arg1 = "message";
	} else if (use_message) {
		char *buffer;
		buffer = strstr(use_message_buffer, "\n\n");
		if (!use_editor && (!buffer || buffer[2] == '\0'))
			die(_("commit has empty message"));
		strbuf_add(&sb, buffer + 2, strlen(buffer + 2));
		hook_arg1 = "commit";
		hook_arg2 = use_message;
	} else if (fixup_message) {
		struct pretty_print_context ctx = {0};
		struct commit *commit;
		commit = lookup_commit_reference_by_name(fixup_message);
		if (!commit)
			die(_("could not lookup commit %s"), fixup_message);
		ctx.output_encoding = get_commit_output_encoding();
		format_commit_message(commit, "fixup! %s\n\n",
				      &sb, &ctx);
		hook_arg1 = "message";
	} else if (!stat(git_path("MERGE_MSG"), &statbuf)) {
		if (strbuf_read_file(&sb, git_path("MERGE_MSG"), 0) < 0)
			die_errno(_("could not read MERGE_MSG"));
		hook_arg1 = "merge";
	} else if (!stat(git_path("SQUASH_MSG"), &statbuf)) {
		if (strbuf_read_file(&sb, git_path("SQUASH_MSG"), 0) < 0)
			die_errno(_("could not read SQUASH_MSG"));
		hook_arg1 = "squash";
	} else if (template_file) {
		if (strbuf_read_file(&sb, template_file, 0) < 0)
			die_errno(_("could not read '%s'"), template_file);
		hook_arg1 = "template";
		clean_message_contents = 0;
	}

	/*
	 * The remaining cases don't modify the template message, but
	 * just set the argument(s) to the prepare-commit-msg hook.
	 */
	else if (whence == FROM_MERGE)
		hook_arg1 = "merge";
	else if (whence == FROM_CHERRY_PICK) {
		hook_arg1 = "commit";
		hook_arg2 = "CHERRY_PICK_HEAD";
	}

	if (squash_message) {
		/*
		 * If squash_commit was used for the commit subject,
		 * then we're possibly hijacking other commit log options.
		 * Reset the hook args to tell the real story.
		 */
		hook_arg1 = "message";
		hook_arg2 = "";
	}

	s->fp = fopen(git_path(commit_editmsg), "w");
	if (s->fp == NULL)
		die_errno(_("could not open '%s'"), git_path(commit_editmsg));

	/* Ignore status.displayCommentPrefix: we do need comments in COMMIT_EDITMSG. */
	old_display_comment_prefix = s->display_comment_prefix;
	s->display_comment_prefix = 1;

	/*
	 * Most hints are counter-productive when the commit has
	 * already started.
	 */
	s->hints = 0;

	if (clean_message_contents)
		stripspace(&sb, 0);

	if (signoff) {
		/*
		 * See if we have a Conflicts: block at the end. If yes, count
		 * its size, so we can ignore it.
		 */
		int ignore_footer = 0;
		int i, eol, previous = 0;
		const char *nl;

		for (i = 0; i < sb.len; i++) {
			nl = memchr(sb.buf + i, '\n', sb.len - i);
			if (nl)
				eol = nl - sb.buf;
			else
				eol = sb.len;
			if (starts_with(sb.buf + previous, "\nConflicts:\n")) {
				ignore_footer = sb.len - previous;
				break;
			}
			while (i < eol)
				i++;
			previous = eol;
		}

		append_signoff(&sb, ignore_footer, 0);
	}

	if (fwrite(sb.buf, 1, sb.len, s->fp) < sb.len)
		die_errno(_("could not write commit template"));

	strbuf_release(&sb);

	/* This checks if committer ident is explicitly given */
	strbuf_addstr(&committer_ident, git_committer_info(IDENT_STRICT));
	if (use_editor && include_status) {
		int ident_shown = 0;
		int saved_color_setting;
		char *ai_tmp, *ci_tmp;
		if (whence != FROM_COMMIT) {
			if (cleanup_mode == CLEANUP_SCISSORS)
				wt_status_add_cut_line(s->fp);
			status_printf_ln(s, GIT_COLOR_NORMAL,
			    whence == FROM_MERGE
				? _("\n"
					"It looks like you may be committing a merge.\n"
					"If this is not correct, please remove the file\n"
					"	%s\n"
					"and try again.\n")
				: _("\n"
					"It looks like you may be committing a cherry-pick.\n"
					"If this is not correct, please remove the file\n"
					"	%s\n"
					"and try again.\n"),
				git_path(whence == FROM_MERGE
					 ? "MERGE_HEAD"
					 : "CHERRY_PICK_HEAD"));
		}

		fprintf(s->fp, "\n");
		if (cleanup_mode == CLEANUP_ALL)
			status_printf(s, GIT_COLOR_NORMAL,
				_("Please enter the commit message for your changes."
				  " Lines starting\nwith '%c' will be ignored, and an empty"
				  " message aborts the commit.\n"), comment_line_char);
		else if (cleanup_mode == CLEANUP_SCISSORS && whence == FROM_COMMIT)
			wt_status_add_cut_line(s->fp);
		else /* CLEANUP_SPACE, that is. */
			status_printf(s, GIT_COLOR_NORMAL,
				_("Please enter the commit message for your changes."
				  " Lines starting\n"
				  "with '%c' will be kept; you may remove them"
				  " yourself if you want to.\n"
				  "An empty message aborts the commit.\n"), comment_line_char);
		if (only_include_assumed)
			status_printf_ln(s, GIT_COLOR_NORMAL,
					"%s", only_include_assumed);

		ai_tmp = cut_ident_timestamp_part(author_ident->buf);
		ci_tmp = cut_ident_timestamp_part(committer_ident.buf);
		if (strcmp(author_ident->buf, committer_ident.buf))
			status_printf_ln(s, GIT_COLOR_NORMAL,
				_("%s"
				"Author:    %s"),
				ident_shown++ ? "" : "\n",
				author_ident->buf);

		if (!committer_ident_sufficiently_given())
			status_printf_ln(s, GIT_COLOR_NORMAL,
				_("%s"
				"Committer: %s"),
				ident_shown++ ? "" : "\n",
				committer_ident.buf);

		if (ident_shown)
			status_printf_ln(s, GIT_COLOR_NORMAL, "");

		saved_color_setting = s->use_color;
		s->use_color = 0;
		commitable = run_status(s->fp, index_file, prefix, 1, s);
		s->use_color = saved_color_setting;

		*ai_tmp = ' ';
		*ci_tmp = ' ';
	} else {
		unsigned char sha1[20];
		const char *parent = "HEAD";

		if (!active_nr && read_cache() < 0)
			die(_("Cannot read index"));

		if (amend)
			parent = "HEAD^1";

		if (get_sha1(parent, sha1))
			commitable = !!active_nr;
		else
			commitable = index_differs_from(parent, 0);
	}
	strbuf_release(&committer_ident);

	fclose(s->fp);

	/*
	 * Reject an attempt to record a non-merge empty commit without
	 * explicit --allow-empty. In the cherry-pick case, it may be
	 * empty due to conflict resolution, which the user should okay.
	 */
	if (!commitable && whence != FROM_MERGE && !allow_empty &&
	    !(amend && is_a_merge(current_head))) {
		s->display_comment_prefix = old_display_comment_prefix;
		run_status(stdout, index_file, prefix, 0, s);
		if (amend)
			fputs(_(empty_amend_advice), stderr);
		else if (whence == FROM_CHERRY_PICK) {
			fputs(_(empty_cherry_pick_advice), stderr);
			if (!sequencer_in_use)
				fputs(_(empty_cherry_pick_advice_single), stderr);
			else
				fputs(_(empty_cherry_pick_advice_multi), stderr);
		}
		return 0;
	}

	/*
	 * Re-read the index as pre-commit hook could have updated it,
	 * and write it out as a tree.  We must do this before we invoke
	 * the editor and after we invoke run_status above.
	 */
	discard_cache();
	read_cache_from(index_file);
	if (update_main_cache_tree(0)) {
		error(_("Error building trees"));
		return 0;
	}

	if (run_commit_hook(use_editor, index_file, "prepare-commit-msg",
			    git_path(commit_editmsg), hook_arg1, hook_arg2, NULL))
		return 0;

	if (use_editor) {
		char index[PATH_MAX];
		const char *env[2] = { NULL };
		env[0] =  index;
		snprintf(index, sizeof(index), "GIT_INDEX_FILE=%s", index_file);
		if (launch_editor(git_path(commit_editmsg), NULL, env)) {
			fprintf(stderr,
			_("Please supply the message using either -m or -F option.\n"));
			exit(1);
		}
	}

	if (!no_verify &&
	    run_commit_hook(use_editor, index_file, "commit-msg", git_path(commit_editmsg), NULL)) {
		return 0;
	}

	return 1;
}