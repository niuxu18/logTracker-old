static void winnt_child_init(apr_pool_t *pchild, struct server_rec *s)
{
    apr_status_t rv;

    setup_signal_names(apr_psprintf(pchild, "ap%lu", parent_pid));

    /* This is a child process, not in single process mode */
    if (!one_process) {
        /* Set up events and the scoreboard */
        get_handles_from_parent(s, &exit_event, &start_mutex,
                                &ap_scoreboard_shm);

        /* Set up the listeners */
        get_listeners_from_parent(s);

        /* Done reading from the parent, close that channel */
        CloseHandle(pipe);
    }
    else {
        /* Single process mode - this lock doesn't even need to exist */
        rv = apr_proc_mutex_create(&start_mutex, signal_name_prefix,
                                   APR_LOCK_DEFAULT, s->process->pool);
        if (rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK,APLOG_ERR, rv, ap_server_conf, APLOGNO(00452)
                         "%s child: Unable to init the start_mutex.",
                         service_name);
            exit(APEXIT_CHILDINIT);
        }

        /* Borrow the shutdown_even as our _child_ loop exit event */
        exit_event = shutdown_event;
    }
}