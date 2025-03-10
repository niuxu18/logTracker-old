int daemon_main(void)
{
	char *pid_file;

	if (is_a_socket(STDIN_FILENO)) {
		int i;

		/* we are running via inetd - close off stdout and
		 * stderr so that library functions (and getopt) don't
		 * try to use them. Redirect them to /dev/null */
		for (i = 1; i < 3; i++) {
			close(i);
			open("/dev/null", O_RDWR);
		}

		return start_daemon(STDIN_FILENO, STDIN_FILENO);
	}

	if (!no_detach)
		become_daemon();

	if (!lp_load(config_file, 1))
		exit_cleanup(RERR_SYNTAX);

	if (rsync_port == 0 && (rsync_port = lp_rsync_port()) == 0)
		rsync_port = RSYNC_PORT;
	if (bind_address == NULL && *lp_bind_address())
		bind_address = lp_bind_address();

	log_init();

	rprintf(FLOG, "rsyncd version %s starting, listening on port %d\n",
		RSYNC_VERSION, rsync_port);
	/* TODO: If listening on a particular address, then show that
	 * address too.  In fact, why not just do inet_ntop on the
	 * local address??? */

	if (((pid_file = lp_pid_file()) != NULL) && (*pid_file != '\0')) {
		char pidbuf[16];
		int fd;
		pid_t pid = getpid();
		cleanup_set_pid(pid);
		if ((fd = do_open(lp_pid_file(), O_WRONLY|O_CREAT|O_TRUNC,
					0666 & ~orig_umask)) == -1) {
			cleanup_set_pid(0);
			rsyserr(FLOG, errno, "failed to create pid file %s",
				pid_file);
			exit_cleanup(RERR_FILEIO);
		}
		snprintf(pidbuf, sizeof pidbuf, "%ld\n", (long)pid);
		write(fd, pidbuf, strlen(pidbuf));
		close(fd);
	}

	start_accept_loop(rsync_port, start_daemon);
	return -1;
}