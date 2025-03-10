static int
next_cache_entry(struct archive_read *a, struct iso9660 *iso9660,
    struct file_info **pfile)
{
	struct file_info *file;
	struct {
		struct file_info	*first;
		struct file_info	**last;
	}	empty_files;
	int64_t number;
	int count;

	file = cache_get_entry(iso9660);
	if (file != NULL) {
		*pfile = file;
		return (ARCHIVE_OK);
	}

	for (;;) {
		struct file_info *re, *d;

		*pfile = file = next_entry(iso9660);
		if (file == NULL) {
			/*
			 * If directory entries all which are descendant of
			 * rr_moved are stil remaning, expose their. 
			 */
			if (iso9660->re_files.first != NULL && 
			    iso9660->rr_moved != NULL &&
			    iso9660->rr_moved->rr_moved_has_re_only)
				/* Expose "rr_moved" entry. */
				cache_add_entry(iso9660, iso9660->rr_moved);
			while ((re = re_get_entry(iso9660)) != NULL) {
				/* Expose its descendant dirs. */
				while ((d = rede_get_entry(re)) != NULL)
					cache_add_entry(iso9660, d);
			}
			if (iso9660->cache_files.first != NULL)
				return (next_cache_entry(a, iso9660, pfile));
			return (ARCHIVE_EOF);
		}

		if (file->cl_offset) {
			struct file_info *first_re = NULL;
			int nexted_re = 0;

			/*
			 * Find "RE" dir for the current file, which
			 * has "CL" flag.
			 */
			while ((re = re_get_entry(iso9660))
			    != first_re) {
				if (first_re == NULL)
					first_re = re;
				if (re->offset == file->cl_offset) {
					re->parent->subdirs--;
					re->parent = file->parent;
					re->re = 0;
					if (re->parent->re_descendant) {
						nexted_re = 1;
						re->re_descendant = 1;
						if (rede_add_entry(re) < 0)
							goto fatal_rr;
						/* Move a list of descendants
						 * to a new ancestor. */
						while ((d = rede_get_entry(
						    re)) != NULL)
							if (rede_add_entry(d)
							    < 0)
								goto fatal_rr;
						break;
					}
					/* Replace the current file
					 * with "RE" dir */
					*pfile = file = re;
					/* Expose its descendant */
					while ((d = rede_get_entry(
					    file)) != NULL)
						cache_add_entry(
						    iso9660, d);
					break;
				} else
					re_add_entry(iso9660, re);
			}
			if (nexted_re) {
				/*
				 * Do not expose this at this time
				 * because we have not gotten its full-path
				 * name yet.
				 */
				continue;
			}
		} else if ((file->mode & AE_IFMT) == AE_IFDIR) {
			int r;

			/* Read file entries in this dir. */
			r = read_children(a, file);
			if (r != ARCHIVE_OK)
				return (r);

			/*
			 * Handle a special dir of Rockridge extensions,
			 * "rr_moved".
			 */
			if (file->rr_moved) {
				/*
				 * If this has only the subdirectories which
				 * have "RE" flags, do not expose at this time.
				 */
				if (file->rr_moved_has_re_only)
					continue;
				/* Otherwise expose "rr_moved" entry. */
			} else if (file->re) {
				/*
				 * Do not expose this at this time
				 * because we have not gotten its full-path
				 * name yet.
				 */
				re_add_entry(iso9660, file);
				continue;
			} else if (file->re_descendant) {
				/*
				 * If the top level "RE" entry of this entry
				 * is not exposed, we, accordingly, should not
				 * expose this entry at this time because
				 * we cannot make its proper full-path name.
				 */
				if (rede_add_entry(file) == 0)
					continue;
				/* Otherwise we can expose this entry because
				 * it seems its top level "RE" has already been
				 * exposed. */
			}
		}
		break;
	}

	if ((file->mode & AE_IFMT) != AE_IFREG || file->number == -1)
		return (ARCHIVE_OK);

	count = 0;
	number = file->number;
	iso9660->cache_files.first = NULL;
	iso9660->cache_files.last = &(iso9660->cache_files.first);
	empty_files.first = NULL;
	empty_files.last = &empty_files.first;
	/* Collect files which has the same file serial number.
	 * Peek pending_files so that file which number is different
	 * is not put bak. */
	while (iso9660->pending_files.used > 0 &&
	    (iso9660->pending_files.files[0]->number == -1 ||
	     iso9660->pending_files.files[0]->number == number)) {
		if (file->number == -1) {
			/* This file has the same offset
			 * but it's wrong offset which empty files
			 * and symlink files have.
			 * NOTE: This wrong offse was recorded by
			 * old mkisofs utility. If ISO images is
			 * created by latest mkisofs, this does not
			 * happen.
			 */
			file->next = NULL;
			*empty_files.last = file;
			empty_files.last = &(file->next);
		} else {
			count++;
			cache_add_entry(iso9660, file);
		}
		file = next_entry(iso9660);
	}

	if (count == 0) {
		*pfile = file;
		return ((file == NULL)?ARCHIVE_EOF:ARCHIVE_OK);
	}
	if (file->number == -1) {
		file->next = NULL;
		*empty_files.last = file;
		empty_files.last = &(file->next);
	} else {
		count++;
		cache_add_entry(iso9660, file);
	}

	if (count > 1) {
		/* The count is the same as number of hardlink,
		 * so much so that each nlinks of files in cache_file
		 * is overwritten by value of the count.
		 */
		for (file = iso9660->cache_files.first;
		    file != NULL; file = file->next)
			file->nlinks = count;
	}
	/* If there are empty files, that files are added
	 * to the tail of the cache_files. */
	if (empty_files.first != NULL) {
		*iso9660->cache_files.last = empty_files.first;
		iso9660->cache_files.last = empty_files.last;
	}
	*pfile = cache_get_entry(iso9660);
	return ((*pfile == NULL)?ARCHIVE_EOF:ARCHIVE_OK);

fatal_rr:
	archive_set_error(&a->archive, ARCHIVE_ERRNO_MISC,
	    "Failed to connect 'CL' pointer to 'RE' rr_moved pointer of"
	    "Rockridge extensions");
	return (ARCHIVE_FATAL);
}