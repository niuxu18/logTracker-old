static int
create_dir(struct archive_write_disk *a, char *path)
{
	struct stat st;
	struct fixup_entry *le;
	char *slash, *base;
	mode_t mode_final, mode;
	int r;

	/* Check for special names and just skip them. */
	slash = strrchr(path, '/');
	if (slash == NULL)
		base = path;
	else
		base = slash + 1;

	if (base[0] == '\0' ||
	    (base[0] == '.' && base[1] == '\0') ||
	    (base[0] == '.' && base[1] == '.' && base[2] == '\0')) {
		/* Don't bother trying to create null path, '.', or '..'. */
		if (slash != NULL) {
			*slash = '\0';
			r = create_dir(a, path);
			*slash = '/';
			return (r);
		}
		return (ARCHIVE_OK);
	}

	/*
	 * Yes, this should be stat() and not lstat().  Using lstat()
	 * here loses the ability to extract through symlinks.  Also note
	 * that this should not use the a->st cache.
	 */
	if (stat(path, &st) == 0) {
		if (S_ISDIR(st.st_mode))
			return (ARCHIVE_OK);
		if ((a->flags & ARCHIVE_EXTRACT_NO_OVERWRITE)) {
			archive_set_error(&a->archive, EEXIST,
			    "Can't create directory '%s'", path);
			return (ARCHIVE_FAILED);
		}
		if (unlink(path) != 0) {
			archive_set_error(&a->archive, errno,
			    "Can't create directory '%s': "
			    "Conflicting file cannot be removed",
			    path);
			return (ARCHIVE_FAILED);
		}
	} else if (errno != ENOENT && errno != ENOTDIR) {
		/* Stat failed? */
		archive_set_error(&a->archive, errno,
		    "Can't test directory '%s'", path);
		return (ARCHIVE_FAILED);
	} else if (slash != NULL) {
		*slash = '\0';
		r = create_dir(a, path);
		*slash = '/';
		if (r != ARCHIVE_OK)
			return (r);
	}

	/*
	 * Mode we want for the final restored directory.  Per POSIX,
	 * implicitly-created dirs must be created obeying the umask.
	 * There's no mention whether this is different for privileged
	 * restores (which the rest of this code handles by pretending
	 * umask=0).  I've chosen here to always obey the user's umask for
	 * implicit dirs, even if _EXTRACT_PERM was specified.
	 */
	mode_final = DEFAULT_DIR_MODE & ~a->user_umask;
	/* Mode we want on disk during the restore process. */
	mode = mode_final;
	mode |= MINIMUM_DIR_MODE;
	mode &= MAXIMUM_DIR_MODE;
	if (mkdir(path, mode) == 0) {
		if (mode != mode_final) {
			le = new_fixup(a, path);
			if (le == NULL)
				return (ARCHIVE_FATAL);
			le->fixup |=TODO_MODE_BASE;
			le->mode = mode_final;
		}
		return (ARCHIVE_OK);
	}

	/*
	 * Without the following check, a/b/../b/c/d fails at the
	 * second visit to 'b', so 'd' can't be created.  Note that we
	 * don't add it to the fixup list here, as it's already been
	 * added.
	 */
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (ARCHIVE_OK);

	archive_set_error(&a->archive, errno, "Failed to create dir '%s'",
	    path);
	return (ARCHIVE_FAILED);
}