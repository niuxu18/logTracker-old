static int netware_query(int query_code, int *result, apr_status_t *rv)
{
    *rv = APR_SUCCESS;
    switch(query_code){
        case AP_MPMQ_MAX_DAEMON_USED:
            *result = 1;
            break;
        case AP_MPMQ_IS_THREADED:
            *result = AP_MPMQ_DYNAMIC;
            break;
        case AP_MPMQ_IS_FORKED:
            *result = AP_MPMQ_NOT_SUPPORTED;
            break;
        case AP_MPMQ_HARD_LIMIT_DAEMONS:
            *result = HARD_SERVER_LIMIT;
            break;
        case AP_MPMQ_HARD_LIMIT_THREADS:
            *result = HARD_THREAD_LIMIT;
            break;
        case AP_MPMQ_MAX_THREADS:
            *result = ap_threads_limit;
            break;
        case AP_MPMQ_MIN_SPARE_DAEMONS:
            *result = 0;
            break;
        case AP_MPMQ_MIN_SPARE_THREADS:
            *result = ap_threads_min_free;
            break;
        case AP_MPMQ_MAX_SPARE_DAEMONS:
            *result = 0;
            break;
        case AP_MPMQ_MAX_SPARE_THREADS:
            *result = ap_threads_max_free;
            break;
        case AP_MPMQ_MAX_REQUESTS_DAEMON:
            *result = ap_max_requests_per_child;
            break;
        case AP_MPMQ_MAX_DAEMONS:
            *result = 1;
            break;
        case AP_MPMQ_MPM_STATE:
            *result = mpm_state;
            break;
        case AP_MPMQ_GENERATION:
            *result = ap_my_generation;
            break;
        default:
            *rv = APR_ENOTIMPL;
            break;
    }
    return OK;
}