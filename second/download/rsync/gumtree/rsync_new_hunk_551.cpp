
	if (preserve_links && S_ISLNK(file->mode)) {
		int l = read_int(f);
		file->link = (char *)malloc(l+1);
		if (!file->link) out_of_memory("receive_file_entry 2");
		read_sbuf(f,file->link,l);
		if (sanitize_paths) {
			sanitize_path(file->link, file->dirname);
		}
	}

#if SUPPORT_HARD_LINKS
	if (preserve_hard_links && S_ISREG(file->mode)) {
		file->dev = read_int(f);
		file->inode = read_int(f);
	}
#endif
  
	if (always_checksum) {
		file->sum = (char *)malloc(MD4_SUM_LENGTH);
		if (!file->sum) out_of_memory("md4 sum");
		if (remote_version < 21) {
			read_buf(f,file->sum,2);
		} else {
			read_buf(f,file->sum,MD4_SUM_LENGTH);
		}
	}
  
	last_mode = file->mode;
	last_rdev = file->rdev;
	last_uid = file->uid;
	last_gid = file->gid;
