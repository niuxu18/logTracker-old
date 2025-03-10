                                   (void *)wl->data, w->pool);
            }
            wl = wl->next;
        }
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, wd_server_conf->s,
                 APLOGNO(02973) "%sWatchdog (%s) stopping",
                 w->singleton ? "Singleton " : "", w->name);

    if (locked)
        apr_proc_mutex_unlock(w->mutex);
    apr_thread_exit(w->thread, APR_SUCCESS);

    return NULL;
