static void get_pack_file(char *name)
{
	select_getanyfile();
	hdr_cache_forever();
	send_local_file("application/x-git-packed-objects", name);
}