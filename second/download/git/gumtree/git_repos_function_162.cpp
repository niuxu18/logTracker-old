static void cache_init(struct submodule_cache *cache)
{
	hashmap_init(&cache->for_path, (hashmap_cmp_fn) config_path_cmp, 0);
	hashmap_init(&cache->for_name, (hashmap_cmp_fn) config_name_cmp, 0);
}