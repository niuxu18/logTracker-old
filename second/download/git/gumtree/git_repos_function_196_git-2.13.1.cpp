static void ensure_cache_init(void)
{
	if (is_cache_init)
		return;

	cache_init(&the_submodule_cache);
	is_cache_init = 1;
}