int copy_xattrs(const char *source, const char *dest)
{
	ssize_t list_len, name_len;
	size_t datum_len;
	char *name, *ptr;
#ifdef HAVE_LINUX_XATTRS
	int user_only = am_root <= 0;
#endif

	/* This puts the name list into the "namebuf" buffer. */
	if ((list_len = get_xattr_names(source)) < 0)
		return -1;

	for (name = namebuf; list_len > 0; name += name_len) {
		name_len = strlen(name) + 1;
		list_len -= name_len;

#ifdef HAVE_LINUX_XATTRS
		/* We always ignore the system namespace, and non-root
		 * ignores everything but the user namespace. */
		if (user_only ? !HAS_PREFIX(name, USER_PREFIX)
			      : HAS_PREFIX(name, SYSTEM_PREFIX))
			continue;
#endif

		datum_len = 0;
		if (!(ptr = get_xattr_data(source, name, &datum_len, 0)))
			return -1;
		if (sys_lsetxattr(dest, name, ptr, datum_len) < 0) {
			int save_errno = errno ? errno : EINVAL;
			rsyserr(FERROR_XFER, errno,
				"rsync_xal_set: lsetxattr(\"%s\",\"%s\") failed",
				dest, name);
			errno = save_errno;
			return -1;
		}
		free(ptr);
	}

	return 0;
}