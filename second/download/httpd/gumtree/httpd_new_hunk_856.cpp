
    apr_pool_create(&ptrans, pchild);
    apr_pool_tag(ptrans, "transaction");

    /* needs to be done before we switch UIDs so we have permissions */
    ap_reopen_scoreboard(pchild, NULL, 0);
    status = apr_proc_mutex_child_init(&accept_mutex, ap_lock_fname, pchild);
    if (status != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_EMERG, status, ap_server_conf,
                     "Couldn't initialize cross-process lock in child "
                     "(%s) (%d)", ap_lock_fname, ap_accept_lock_mech);
        clean_child_exit(APEXIT_CHILDFATAL);
    }

    if (unixd_setup_child()) {
	clean_child_exit(APEXIT_CHILDFATAL);
    }
