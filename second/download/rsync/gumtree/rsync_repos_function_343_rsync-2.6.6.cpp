static struct file_struct *receive_file_entry(struct file_list *flist,
					      unsigned short flags, int f)
{
	static time_t modtime;
	static mode_t mode;
	static int64 dev;
	static dev_t rdev;
	static uint32 rdev_major;
	static uid_t uid;
	static gid_t gid;
	static char lastname[MAXPATHLEN], *lastdir;
	static int lastdir_depth, lastdir_len = -1;
	static unsigned int del_hier_name_len = 0;
	static int in_del_hier = 0;
	char thisname[MAXPATHLEN];
	unsigned int l1 = 0, l2 = 0;
	int alloc_len, basename_len, dirname_len, linkname_len, sum_len;
	OFF_T file_length;
	char *basename, *dirname, *bp;
	struct file_struct *file;

	if (!flist) {
		modtime = 0, mode = 0;
		dev = 0, rdev = makedev(0, 0);
		rdev_major = 0;
		uid = 0, gid = 0;
		*lastname = '\0';
		lastdir_len = -1;
		in_del_hier = 0;
		return NULL;
	}

	if (flags & XMIT_SAME_NAME)
		l1 = read_byte(f);

	if (flags & XMIT_LONG_NAME)
		l2 = read_int(f);
	else
		l2 = read_byte(f);

	if (l2 >= MAXPATHLEN - l1) {
		rprintf(FERROR,
			"overflow: flags=0x%x l1=%d l2=%d lastname=%s\n",
			flags, l1, l2, safe_fname(lastname));
		overflow_exit("receive_file_entry");
	}

	strlcpy(thisname, lastname, l1 + 1);
	read_sbuf(f, &thisname[l1], l2);
	thisname[l1 + l2] = 0;

	strlcpy(lastname, thisname, MAXPATHLEN);

	clean_fname(thisname, 0);

	if (sanitize_paths)
		sanitize_path(thisname, thisname, "", 0);

	if ((basename = strrchr(thisname, '/')) != NULL) {
		dirname_len = ++basename - thisname; /* counts future '\0' */
		if (lastdir_len == dirname_len - 1
		    && strncmp(thisname, lastdir, lastdir_len) == 0) {
			dirname = lastdir;
			dirname_len = 0; /* indicates no copy is needed */
		} else
			dirname = thisname;
	} else {
		basename = thisname;
		dirname = NULL;
		dirname_len = 0;
	}
	basename_len = strlen(basename) + 1; /* count the '\0' */

	file_length = read_longint(f);
	if (!(flags & XMIT_SAME_TIME))
		modtime = (time_t)read_int(f);
	if (!(flags & XMIT_SAME_MODE))
		mode = from_wire_mode(read_int(f));

	if (preserve_uid && !(flags & XMIT_SAME_UID))
		uid = (uid_t)read_int(f);
	if (preserve_gid && !(flags & XMIT_SAME_GID))
		gid = (gid_t)read_int(f);

	if (preserve_devices) {
		if (protocol_version < 28) {
			if (IS_DEVICE(mode)) {
				if (!(flags & XMIT_SAME_RDEV_pre28))
					rdev = (dev_t)read_int(f);
			} else
				rdev = makedev(0, 0);
		} else if (IS_DEVICE(mode)) {
			uint32 rdev_minor;
			if (!(flags & XMIT_SAME_RDEV_MAJOR))
				rdev_major = read_int(f);
			if (flags & XMIT_RDEV_MINOR_IS_SMALL)
				rdev_minor = read_byte(f);
			else
				rdev_minor = read_int(f);
			rdev = makedev(rdev_major, rdev_minor);
		}
	}

#ifdef SUPPORT_LINKS
	if (preserve_links && S_ISLNK(mode)) {
		linkname_len = read_int(f) + 1; /* count the '\0' */
		if (linkname_len <= 0 || linkname_len > MAXPATHLEN) {
			rprintf(FERROR, "overflow: linkname_len=%d\n",
				linkname_len - 1);
			overflow_exit("receive_file_entry");
		}
	}
	else
#endif
		linkname_len = 0;

	sum_len = always_checksum && S_ISREG(mode) ? MD4_SUM_LENGTH : 0;

	alloc_len = file_struct_len + dirname_len + basename_len
		  + linkname_len + sum_len;
	bp = pool_alloc(flist->file_pool, alloc_len, "receive_file_entry");

	file = (struct file_struct *)bp;
	memset(bp, 0, file_struct_len);
	bp += file_struct_len;

	file->flags = 0;
	file->modtime = modtime;
	file->length = file_length;
	file->mode = mode;
	file->uid = uid;
	file->gid = gid;

	if (dirname_len) {
		file->dirname = lastdir = bp;
		lastdir_len = dirname_len - 1;
		memcpy(bp, dirname, dirname_len - 1);
		bp += dirname_len;
		bp[-1] = '\0';
		lastdir_depth = count_dir_elements(lastdir);
		file->dir.depth = lastdir_depth + 1;
	} else if (dirname) {
		file->dirname = dirname; /* we're reusing lastname */
		file->dir.depth = lastdir_depth + 1;
	} else
		file->dir.depth = 1;

	if (S_ISDIR(mode)) {
		if (basename_len == 1+1 && *basename == '.') /* +1 for '\0' */
			file->dir.depth--;
		if (flags & XMIT_TOP_DIR) {
			in_del_hier = 1;
			del_hier_name_len = file->dir.depth == 0 ? 0 : l1 + l2;
			if (relative_paths && del_hier_name_len > 2
			    && basename_len == 1+1 && *basename == '.')
				del_hier_name_len -= 2;
			file->flags |= FLAG_TOP_DIR | FLAG_DEL_HERE;
		} else if (in_del_hier) {
			if (!relative_paths || !del_hier_name_len
			 || (l1 >= del_hier_name_len
			  && thisname[del_hier_name_len] == '/'))
				file->flags |= FLAG_DEL_HERE;
			else
				in_del_hier = 0;
		}
	}

	file->basename = bp;
	memcpy(bp, basename, basename_len);
	bp += basename_len;

	if (preserve_devices && IS_DEVICE(mode))
		file->u.rdev = rdev;

#ifdef SUPPORT_LINKS
	if (linkname_len) {
		file->u.link = bp;
		read_sbuf(f, bp, linkname_len - 1);
		if (sanitize_paths)
			sanitize_path(bp, bp, "", lastdir_depth);
		bp += linkname_len;
	}
#endif

#ifdef SUPPORT_HARD_LINKS
	if (preserve_hard_links && protocol_version < 28 && S_ISREG(mode))
		flags |= XMIT_HAS_IDEV_DATA;
	if (flags & XMIT_HAS_IDEV_DATA) {
		int64 inode;
		if (protocol_version < 26) {
			dev = read_int(f);
			inode = read_int(f);
		} else {
			if (!(flags & XMIT_SAME_DEV))
				dev = read_longint(f);
			inode = read_longint(f);
		}
		if (flist->hlink_pool) {
			file->link_u.idev = pool_talloc(flist->hlink_pool,
			    struct idev, 1, "inode_table");
			file->F_INODE = inode;
			file->F_DEV = dev;
		}
	}
#endif

	if (always_checksum) {
		char *sum;
		if (sum_len) {
			file->u.sum = sum = bp;
			/*bp += sum_len;*/
		} else if (protocol_version < 28) {
			/* Prior to 28, we get a useless set of nulls. */
			sum = empty_sum;
		} else
			sum = NULL;
		if (sum) {
			read_buf(f, sum,
			    protocol_version < 21 ? 2 : MD4_SUM_LENGTH);
		}
	}

	if (!preserve_perms) {
		/* set an appropriate set of permissions based on original
		 * permissions and umask. This emulates what GNU cp does */
		file->mode &= ~orig_umask;
	}

	return file;
}