static int fsck_subdir(int nr, const char *path, void *progress)
{
	display_progress(progress, nr + 1);
	return 0;
}