static int verify_absent_1(const struct cache_entry *ce,
			   enum unpack_trees_error_types error_type,
			   struct unpack_trees_options *o)
{
	int len;
	struct stat st;

	if (o->index_only || o->reset || !o->update)
		return 0;

	len = check_leading_path(ce->name, ce_namelen(ce));
	if (!len)
		return 0;
	else if (len > 0) {
		char path[PATH_MAX + 1];
		memcpy(path, ce->name, len);
		path[len] = 0;
		if (lstat(path, &st))
			return error("cannot stat '%s': %s", path,
					strerror(errno));

		return check_ok_to_remove(path, len, DT_UNKNOWN, NULL, &st,
				error_type, o);
	} else if (lstat(ce->name, &st)) {
		if (errno != ENOENT)
			return error("cannot stat '%s': %s", ce->name,
				     strerror(errno));
		return 0;
	} else {
		return check_ok_to_remove(ce->name, ce_namelen(ce),
					  ce_to_dtype(ce), ce, &st,
					  error_type, o);
	}
}