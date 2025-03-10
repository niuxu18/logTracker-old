void _exit_cleanup(int code, const char *file, int line)
{
	int ocode = code;
	extern int keep_partial;
	extern int log_got_error;

	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	if (verbose > 3)
		rprintf(FINFO,"_exit_cleanup(code=%d, file=%s, line=%d): entered\n", 
			code, file, line);

	if (cleanup_child_pid != -1) {
		int status;
		if (waitpid(cleanup_child_pid, &status, WNOHANG) == cleanup_child_pid) {
			status = WEXITSTATUS(status);
			if (status > code) code = status;
		}
	}

	if (cleanup_got_literal && cleanup_fname && keep_partial) {
		char *fname = cleanup_fname;
		cleanup_fname = NULL;
		if (cleanup_buf) unmap_file(cleanup_buf);
		if (cleanup_fd1 != -1) close(cleanup_fd1);
		if (cleanup_fd2 != -1) close(cleanup_fd2);
		finish_transfer(cleanup_new_fname, fname, cleanup_file);
	}
	io_flush();
	if (cleanup_fname)
		do_unlink(cleanup_fname);
	if (code) {
		kill_all(SIGUSR1);
	}
	if ((cleanup_pid != 0) && (cleanup_pid == (int) getpid())) {
		char *pidf = lp_pid_file();
		if (pidf && *pidf) {
			unlink(lp_pid_file());
		}
	}

	if (code == 0 && (io_error || log_got_error)) {
		code = RERR_PARTIAL;
	}

	if (code) log_exit(code, file, line);

	if (verbose > 2)
		rprintf(FINFO,"_exit_cleanup(code=%d, file=%s, line=%d): about to call exit(%d)\n", 
			ocode, file, line, code);

	exit(code);
}