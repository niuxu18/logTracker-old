(strlen(tail) >= PATH_MAX) {
		/* Locate a dir prefix shorter than PATH_MAX. */
		tail += PATH_MAX - 8;
		while (tail > a->name && *tail != '/')
			tail--;
		/* Exit if we find a too-long path component. */
		if (tail <= a->name)
			return;
		/* Create the intermediate dir and chdir to it. */
		*tail = '\0'; /* Terminate dir portion */
		ret = create_dir(a, a->name);
		if (ret == ARCHIVE_OK && chdir(a->name) != 0)
			ret = ARCHIVE_FAILED;
		*tail = '/'; /* Restore the / we removed. */
		if (ret != ARCHIVE_OK)
			return;
		tail++;
		/* The chdir() succeeded; we've now shortened the path. */
		a->name = tail;
	}