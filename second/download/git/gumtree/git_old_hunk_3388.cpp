		contents_sz = xsize_t(st.st_size);
		contents = xmmap_gently(NULL, contents_sz, PROT_READ,
					MAP_PRIVATE, in_fd, 0);
		if (contents == MAP_FAILED) {
			if (errno == ENODEV && S_ISDIR(st.st_mode))
				errno = EISDIR;
			error("unable to mmap '%s': %s",
			      config_filename, strerror(errno));
			ret = CONFIG_INVALID_FILE;
			contents = NULL;
			goto out_free;
		}
		close(in_fd);
		in_fd = -1;

		if (chmod(get_lock_file_path(lock), st.st_mode & 07777) < 0) {
			error("chmod on %s failed: %s",
			      get_lock_file_path(lock), strerror(errno));
			ret = CONFIG_NO_WRITE;
			goto out_free;
		}

		if (store.seen == 0)
			store.seen = 1;
