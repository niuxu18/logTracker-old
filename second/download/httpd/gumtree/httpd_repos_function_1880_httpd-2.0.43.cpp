static const char *set_limit_mem(cmd_parms *cmd, void *conf_,
                                 const char *arg, const char * arg2)
{
    core_dir_config *conf = conf_;

#if defined(RLIMIT_AS)
    unixd_set_rlimit(cmd, &conf->limit_mem, arg, arg2 ,RLIMIT_AS);
#elif defined(RLIMIT_DATA)
    unixd_set_rlimit(cmd, &conf->limit_mem, arg, arg2, RLIMIT_DATA);
#elif defined(RLIMIT_VMEM)
    unixd_set_rlimit(cmd, &conf->limit_mem, arg, arg2, RLIMIT_VMEM);
#endif

    return NULL;
}