static int make_child(server_rec *s, int slot) 
{
    int pid;

    if (slot + 1 > ap_max_daemons_limit) {
        ap_max_daemons_limit = slot + 1;
    }

    if (one_process) {
        set_signals();
        ap_scoreboard_image->parent[slot].pid = getpid();
        child_main(slot);
    }

    if ((pid = fork()) == -1) {
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, s, 
                     "fork: Unable to fork new process");

        /* fork didn't succeed. Fix the scoreboard or else
         * it will say SERVER_STARTING forever and ever
         */
        ap_update_child_status_from_indexes(slot, 0, SERVER_DEAD, NULL);

        /* In case system resources are maxxed out, we don't want
           Apache running away with the CPU trying to fork over and
           over and over again. */
        apr_sleep(10 * APR_USEC_PER_SEC);

        return -1;
    }

    if (!pid) {
#ifdef HAVE_BINDPROCESSOR
        /* By default, AIX binds to a single processor.  This bit unbinds
         * children which will then bind to another CPU.
         */
        int status = bindprocessor(BINDPROCESS, (int)getpid(),
                               PROCESSOR_CLASS_ANY);
        if (status != OK)
            ap_log_error(APLOG_MARK, APLOG_WARNING, errno, 
                         ap_server_conf,
                         "processor unbind failed %d", status);
#endif
        RAISE_SIGSTOP(MAKE_CHILD);

        apr_signal(SIGTERM, just_die);
        child_main(slot);

        clean_child_exit(0);
    }
    /* else */
    ap_scoreboard_image->parent[slot].quiescing = 0;
    ap_scoreboard_image->parent[slot].pid = pid;
    return 0;
}