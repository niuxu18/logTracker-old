static int write_entry(struct cache_entry *ce,
		       char *path, const struct checkout *state, int to_tempfile)
{
	unsigned int ce_mode_s_ifmt = ce->ce_mode & S_IFMT;
	int fd, ret, fstat_done = 0;
	char *new;
	struct strbuf buf = STRBUF_INIT;
	unsigned long size;
	size_t wrote, newsize = 0;
	struct stat st;

	if (ce_mode_s_ifmt == S_IFREG) {
		struct stream_filter *filter = get_stream_filter(ce->name, ce->sha1);
		if (filter &&
		    !streaming_write_entry(ce, path, filter,
					   state, to_tempfile,
					   &fstat_done, &st))
			goto finish;
	}

	switch (ce_mode_s_ifmt) {
	case S_IFREG:
	case S_IFLNK:
		new = read_blob_entry(ce, &size);
		if (!new)
			return error("unable to read sha1 file of %s (%s)",
				path, sha1_to_hex(ce->sha1));

		if (ce_mode_s_ifmt == S_IFLNK && has_symlinks && !to_tempfile) {
			ret = symlink(new, path);
			free(new);
			if (ret)
				return error("unable to create symlink %s (%s)",
					     path, strerror(errno));
			break;
		}

		/*
		 * Convert from git internal format to working tree format
		 */
		if (ce_mode_s_ifmt == S_IFREG &&
		    convert_to_working_tree(ce->name, new, size, &buf)) {
			free(new);
			new = strbuf_detach(&buf, &newsize);
			size = newsize;
		}

		fd = open_output_fd(path, ce, to_tempfile);
		if (fd < 0) {
			free(new);
			return error("unable to create file %s (%s)",
				path, strerror(errno));
		}

		wrote = write_in_full(fd, new, size);
		if (!to_tempfile)
			fstat_done = fstat_output(fd, state, &st);
		close(fd);
		free(new);
		if (wrote != size)
			return error("unable to write file %s", path);
		break;
	case S_IFGITLINK:
		if (to_tempfile)
			return error("cannot create temporary submodule %s", path);
		if (mkdir(path, 0777) < 0)
			return error("cannot create submodule directory %s", path);
		break;
	default:
		return error("unknown file mode for %s in index", path);
	}

finish:
	if (state->refresh_cache) {
		if (!fstat_done)
			lstat(ce->name, &st);
		fill_stat_cache_info(ce, &st);
	}
	return 0;
}