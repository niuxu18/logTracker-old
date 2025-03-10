static const char *real_path_internal(const char *path, int die_on_error)
{
	static struct strbuf sb = STRBUF_INIT;
	char *retval = NULL;

	/*
	 * If we have to temporarily chdir(), store the original CWD
	 * here so that we can chdir() back to it at the end of the
	 * function:
	 */
	struct strbuf cwd = STRBUF_INIT;

	int depth = MAXDEPTH;
	char *last_elem = NULL;
	struct stat st;

	/* We've already done it */
	if (path == sb.buf)
		return path;

	if (!*path) {
		if (die_on_error)
			die("The empty string is not a valid path");
		else
			goto error_out;
	}

	strbuf_reset(&sb);
	strbuf_addstr(&sb, path);

	while (depth--) {
		if (!is_directory(sb.buf)) {
			char *last_slash = find_last_dir_sep(sb.buf);
			if (last_slash) {
				last_elem = xstrdup(last_slash + 1);
				strbuf_setlen(&sb, last_slash - sb.buf + 1);
			} else {
				last_elem = xmemdupz(sb.buf, sb.len);
				strbuf_reset(&sb);
			}
		}

		if (sb.len) {
			if (!cwd.len && strbuf_getcwd(&cwd)) {
				if (die_on_error)
					die_errno("Could not get current working directory");
				else
					goto error_out;
			}

			if (chdir(sb.buf)) {
				if (die_on_error)
					die_errno("Could not switch to '%s'",
						  sb.buf);
				else
					goto error_out;
			}
		}
		if (strbuf_getcwd(&sb)) {
			if (die_on_error)
				die_errno("Could not get current working directory");
			else
				goto error_out;
		}

		if (last_elem) {
			if (sb.len && !is_dir_sep(sb.buf[sb.len - 1]))
				strbuf_addch(&sb, '/');
			strbuf_addstr(&sb, last_elem);
			free(last_elem);
			last_elem = NULL;
		}

		if (!lstat(sb.buf, &st) && S_ISLNK(st.st_mode)) {
			struct strbuf next_sb = STRBUF_INIT;
			ssize_t len = strbuf_readlink(&next_sb, sb.buf, 0);
			if (len < 0) {
				if (die_on_error)
					die_errno("Invalid symlink '%s'",
						  sb.buf);
				else
					goto error_out;
			}
			strbuf_swap(&sb, &next_sb);
			strbuf_release(&next_sb);
		} else
			break;
	}

	retval = sb.buf;
error_out:
	free(last_elem);
	if (cwd.len && chdir(cwd.buf))
		die_errno("Could not change back to '%s'", cwd.buf);
	strbuf_release(&cwd);

	return retval;
}