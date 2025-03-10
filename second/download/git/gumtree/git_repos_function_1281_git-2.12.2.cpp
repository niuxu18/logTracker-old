int apply_all_patches(struct apply_state *state,
		      int argc,
		      const char **argv,
		      int options)
{
	int i;
	int res;
	int errs = 0;
	int read_stdin = 1;

	for (i = 0; i < argc; i++) {
		const char *arg = argv[i];
		int fd;

		if (!strcmp(arg, "-")) {
			res = apply_patch(state, 0, "<stdin>", options);
			if (res < 0)
				goto end;
			errs |= res;
			read_stdin = 0;
			continue;
		} else if (0 < state->prefix_length)
			arg = prefix_filename(state->prefix,
					      state->prefix_length,
					      arg);

		fd = open(arg, O_RDONLY);
		if (fd < 0) {
			error(_("can't open patch '%s': %s"), arg, strerror(errno));
			res = -128;
			goto end;
		}
		read_stdin = 0;
		set_default_whitespace_mode(state);
		res = apply_patch(state, fd, arg, options);
		close(fd);
		if (res < 0)
			goto end;
		errs |= res;
	}
	set_default_whitespace_mode(state);
	if (read_stdin) {
		res = apply_patch(state, 0, "<stdin>", options);
		if (res < 0)
			goto end;
		errs |= res;
	}

	if (state->whitespace_error) {
		if (state->squelch_whitespace_errors &&
		    state->squelch_whitespace_errors < state->whitespace_error) {
			int squelched =
				state->whitespace_error - state->squelch_whitespace_errors;
			warning(Q_("squelched %d whitespace error",
				   "squelched %d whitespace errors",
				   squelched),
				squelched);
		}
		if (state->ws_error_action == die_on_ws_error) {
			error(Q_("%d line adds whitespace errors.",
				 "%d lines add whitespace errors.",
				 state->whitespace_error),
			      state->whitespace_error);
			res = -128;
			goto end;
		}
		if (state->applied_after_fixing_ws && state->apply)
			warning(Q_("%d line applied after"
				   " fixing whitespace errors.",
				   "%d lines applied after"
				   " fixing whitespace errors.",
				   state->applied_after_fixing_ws),
				state->applied_after_fixing_ws);
		else if (state->whitespace_error)
			warning(Q_("%d line adds whitespace errors.",
				   "%d lines add whitespace errors.",
				   state->whitespace_error),
				state->whitespace_error);
	}

	if (state->update_index) {
		res = write_locked_index(&the_index, state->lock_file, COMMIT_LOCK);
		if (res) {
			error(_("Unable to write new index file"));
			res = -128;
			goto end;
		}
		state->newfd = -1;
	}

	res = !!errs;

end:
	if (state->newfd >= 0) {
		rollback_lock_file(state->lock_file);
		state->newfd = -1;
	}

	if (state->apply_verbosity <= verbosity_silent) {
		set_error_routine(state->saved_error_routine);
		set_warn_routine(state->saved_warn_routine);
	}

	if (res > -1)
		return res;
	return (res == -1 ? 1 : 128);
}