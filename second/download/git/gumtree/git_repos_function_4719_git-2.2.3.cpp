static int show_tree_object(const unsigned char *sha1,
		const char *base, int baselen,
		const char *pathname, unsigned mode, int stage, void *context)
{
	printf("%s%s\n", pathname, S_ISDIR(mode) ? "/" : "");
	return 0;
}