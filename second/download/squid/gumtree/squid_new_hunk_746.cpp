    if (Config.chroot_dir && (geteuid() == 0)) {
        snprintf(pathbuf, BUFSIZ, "%s/%s", Config.chroot_dir, path);
        path = pathbuf;
    }

    if (stat(path, &sb) < 0) {
        debugs(0, DBG_CRITICAL, (opt_parse_cfg_only?"FATAL: ":"ERROR: ") << name << " " << path << ": " << xstrerror());
        // keep going to find more issues if we are only checking the config file with "-k parse"
        if (opt_parse_cfg_only)
            return;
        // this is fatal if it is found during startup or reconfigure
        if (opt_send_signal == -1 || opt_send_signal == SIGHUP)
            fatalf("%s %s: %s", name, path, xstrerror());
