int sock_exec(const char *prog)
{
	int fd[2];

	if (socketpair_tcp(fd) != 0) {
		rsyserr(FERROR, errno, "socketpair_tcp failed");
		return -1;
	}
	if (verbose >= 2)
		rprintf(FINFO, "Running socket program: \"%s\"\n", prog);
	if (fork() == 0) {
		close(fd[0]);
		close(0);
		close(1);
		dup(fd[1]);
		dup(fd[1]);
		exit(system(prog));
	}
	close(fd[1]);
	return fd[0];
}