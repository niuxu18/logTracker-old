
#ifdef HAVE_PUTENV
	if (*lp_prexfer_exec(i) || *lp_postxfer_exec(i)) {
		char *modname, *modpath, *hostaddr, *hostname, *username;
		int status;

		if (!use_chroot)
			p = module_dir;
		else if (module_dirlen) {
			pathjoin(line, sizeof line, chroot_path, module_dir+1);
			p = line;
		} else
			p = chroot_path;

		if (asprintf(&modname, "RSYNC_MODULE_NAME=%s", name) < 0
		 || asprintf(&modpath, "RSYNC_MODULE_PATH=%s", p) < 0
		 || asprintf(&hostaddr, "RSYNC_HOST_ADDR=%s", addr) < 0
		 || asprintf(&hostname, "RSYNC_HOST_NAME=%s", host) < 0
		 || asprintf(&username, "RSYNC_USER_NAME=%s", auth_user) < 0)
			out_of_memory("rsync_module");
		putenv(modname);
		putenv(modpath);
