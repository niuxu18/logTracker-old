                     (startup ? NULL : s),
                     "no listening sockets available, shutting down");
        return DONE;
    }

    if (!one_process) {
        if ((rv = ap_mpm_podx_open(pconf, &pod))) {
            ap_log_error(APLOG_MARK, APLOG_CRIT | level_flags, rv,
                         (startup ? NULL : s),
                         "could not open pipe-of-death");
            return DONE;
        }
    }
    /* for skiplist */
    srand((unsigned int)apr_time_now());
    return OK;
}

static int event_pre_config(apr_pool_t * pconf, apr_pool_t * plog,
                            apr_pool_t * ptemp)
{
