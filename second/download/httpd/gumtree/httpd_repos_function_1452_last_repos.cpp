static void *create_config(apr_pool_t *p, server_rec *s)
{
    disk_cache_conf *conf = apr_pcalloc(p, sizeof(disk_cache_conf));

    /* XXX: Set default values */
    conf->dirlevels = DEFAULT_DIRLEVELS;
    conf->dirlength = DEFAULT_DIRLENGTH;

    conf->cache_root = NULL;
    conf->cache_root_len = 0;

    return conf;
}