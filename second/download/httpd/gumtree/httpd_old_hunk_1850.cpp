                ap_mpm_safe_kill(ap_scoreboard_image->parent[index].pid, AP_SIG_GRACEFUL);
            }
        }
    }
    else {
        /* Kill 'em off */
        if (unixd_killpg(getpgrp(), SIGHUP) < 0) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, errno, ap_server_conf, "killpg SIGHUP");
        }
        ap_reclaim_child_processes(0);          /* Not when just starting up */
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf,
                    "SIGHUP received.  Attempting to restart");
    }

    return 0;
}

/* This really should be a post_config hook, but the error log is already
 * redirected by that point, so we need to do this in the open_logs phase.
 */
static int prefork_open_logs(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp, server_rec *s)
{
    apr_status_t rv;

    pconf = p;
    ap_server_conf = s;

    if ((num_listensocks = ap_setup_listeners(ap_server_conf)) < 1) {
        ap_log_error(APLOG_MARK, APLOG_ALERT|APLOG_STARTUP, 0,
                     NULL, "no listening sockets available, shutting down");
        return DONE;
    }

    if ((rv = ap_mpm_pod_open(pconf, &pod))) {
        ap_log_error(APLOG_MARK, APLOG_CRIT|APLOG_STARTUP, rv, NULL,
                "Could not open pipe-of-death.");
        return DONE;
    }
    return OK;
}

static int prefork_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    static int restart_num = 0;
    int no_detach, debug, foreground;
    apr_status_t rv;

    mpm_state = AP_MPMQ_STARTING;

    debug = ap_exists_config_define("DEBUG");

    if (debug) {
