void cleanup_set(char *fnametmp, char *fname, struct file_struct *file,
		 int fd_r, int fd_w)
{
	cleanup_fname = fnametmp;
	cleanup_new_fname = fname;
	cleanup_file = file;
	cleanup_fd_r = fd_r;
	cleanup_fd_w = fd_w;
}