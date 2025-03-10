	setuid(unixd_config.user_id) == -1)) {
	ap_log_error(APLOG_MARK, APLOG_ALERT, errno, NULL,
		    "setuid: unable to change to uid: %ld",
                    (long) unixd_config.user_id);
	return -1;
    }
#if defined(HAVE_PRCTL) && defined(PR_SET_DUMPABLE) 
    /* this applies to Linux 2.4+ */
#ifdef AP_MPM_WANT_SET_COREDUMPDIR
    if (ap_coredumpdir_configured) {
        if (prctl(PR_SET_DUMPABLE, 1)) {
            ap_log_error(APLOG_MARK, APLOG_ALERT, errno, NULL,
                         "set dumpable failed - this child will not coredump"
                         " after software errors");
        }
    }
#endif
#endif
#endif
    return 0;
}


AP_DECLARE(const char *) unixd_set_user(cmd_parms *cmd, void *dummy, 
