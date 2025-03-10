static void
mainSetCwd(void)
{
    char pathbuf[MAXPATHLEN];

    if (Config.coredump_dir) {
        if (0 == strcmp("none", Config.coredump_dir)) {
            (void) 0;
        } else if (chdir(Config.coredump_dir) == 0) {
            debugs(0, DBG_IMPORTANT, "Set Current Directory to " << Config.coredump_dir);
            return;
        } else {
            debugs(50, DBG_CRITICAL, "chdir: " << Config.coredump_dir << ": " << xstrerror());
        }
    }

    /* If we don't have coredump_dir or couldn't cd there, report current dir */
    if (getcwd(pathbuf, MAXPATHLEN)) {
        debugs(0, DBG_IMPORTANT, "Current Directory is " << pathbuf);
    } else {
        debugs(50, DBG_CRITICAL, "WARNING: Can't find current directory, getcwd: " << xstrerror());
    }
}