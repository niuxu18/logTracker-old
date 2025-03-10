            cleanup_thread(child_handles, &threads_created, watch_thread);
        }
    }

    /* Kill remaining threads off the hard way */
    if (threads_created) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, APR_SUCCESS, ap_server_conf, APLOGNO(00363)
                     "Child: Terminating %d threads that failed to exit.",
                     threads_created);
    }
    for (i = 0; i < threads_created; i++) {
        int *score_idx;
        TerminateThread(child_handles[i], 1);
        CloseHandle(child_handles[i]);
        /* Reset the scoreboard entry for the thread we just whacked */
        score_idx = apr_hash_get(ht, &child_handles[i], sizeof(HANDLE));
        if (score_idx) {
            ap_update_child_status_from_indexes(0, *score_idx, SERVER_DEAD, NULL);
        }
    }
    ap_log_error(APLOG_MARK, APLOG_NOTICE, APR_SUCCESS, ap_server_conf, APLOGNO(00364)
                 "Child: All worker threads have exited.");

    apr_thread_mutex_destroy(child_lock);
    apr_thread_mutex_destroy(qlock);
    CloseHandle(qwait_event);

    apr_pool_destroy(pchild);
