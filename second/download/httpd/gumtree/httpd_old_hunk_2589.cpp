}

#else /* not NO_DLOPEN */

static const char *load_file(cmd_parms *cmd, void *dummy, const char *filename)
{
    ap_log_perror(APLOG_MARK, APLOG_STARTUP, 0, cmd->pool,
                 "WARNING: LoadFile not supported on this platform");
    return NULL;
}

static const char *load_module(cmd_parms *cmd, void *dummy,
                               const char *modname, const char *filename)
{
    ap_log_perror(APLOG_MARK, APLOG_STARTUP, 0, cmd->pool,
                 "WARNING: LoadModule not supported on this platform");
    return NULL;
}

#endif /* NO_DLOPEN */

