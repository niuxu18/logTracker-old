static int
add_entry(struct archive_match *a, int flag,
    struct archive_entry *entry)
{
	struct match_file *f;
	const void *pathname;
	int r;

	f = calloc(1, sizeof(*f));
	if (f == NULL)
		return (error_nomem(a));

#if defined(_WIN32) && !defined(__CYGWIN__)
	pathname = archive_entry_pathname_w(entry);
	if (pathname == NULL) {
		free(f);
		archive_set_error(&(a->archive), EINVAL, "pathname is NULL");
		return (ARCHIVE_FAILED);
	}
	archive_mstring_copy_wcs(&(f->pathname), pathname);
	a->exclusion_tree.rbt_ops = &rb_ops_wcs;
#else
	(void)rb_ops_wcs;
	pathname = archive_entry_pathname(entry);
	if (pathname == NULL) {
		free(f);
		archive_set_error(&(a->archive), EINVAL, "pathname is NULL");
		return (ARCHIVE_FAILED);
	}
	archive_mstring_copy_mbs(&(f->pathname), pathname);
	a->exclusion_tree.rbt_ops = &rb_ops_mbs;
#endif
	f->flag = flag;
	f->mtime_sec = archive_entry_mtime(entry);
	f->mtime_nsec = archive_entry_mtime_nsec(entry);
	f->ctime_sec = archive_entry_ctime(entry);
	f->ctime_nsec = archive_entry_ctime_nsec(entry);
	r = __archive_rb_tree_insert_node(&(a->exclusion_tree), &(f->node));
	if (!r) {
		struct match_file *f2;

		/* Get the duplicated file. */
		f2 = (struct match_file *)__archive_rb_tree_find_node(
			&(a->exclusion_tree), pathname);

		/*
		 * We always overwrite comparison condition.
		 * If you do not want to overwrite it, you should not
		 * call archive_match_exclude_entry(). We cannot know
		 * what behavior you really expect since overwriting
		 * condition might be different with the flag.
		 */
		if (f2 != NULL) {
			f2->flag = f->flag;
			f2->mtime_sec = f->mtime_sec;
			f2->mtime_nsec = f->mtime_nsec;
			f2->ctime_sec = f->ctime_sec;
			f2->ctime_nsec = f->ctime_nsec;
		}
		/* Release the duplicated file. */
		archive_mstring_clean(&(f->pathname));
		free(f);
		return (ARCHIVE_OK);
	}
	entry_list_add(&(a->exclusion_entry_list), f);
	a->setflag |= TIME_IS_SET;
	return (ARCHIVE_OK);
}