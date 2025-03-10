static int ce_compare_data(const struct cache_entry *ce, struct stat *st)
{
	int match = -1;
	int fd = open(ce->name, O_RDONLY);

	if (fd >= 0) {
		unsigned char sha1[20];
		if (!index_fd(sha1, fd, st, OBJ_BLOB, ce->name, 0))
			match = hashcmp(sha1, ce->sha1);
		/* index_fd() closed the file descriptor already */
	}
	return match;
}