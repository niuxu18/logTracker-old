/* service_nt_main_fn is outside of the call stack and outside of the
 * primary server thread... so now we _really_ need a placeholder!
 * The winnt_rewrite_args has created and shared mpm_new_argv with us.
 */
extern apr_array_header_t *mpm_new_argv;

/* ###: utf-ize */
static void __stdcall service_nt_main_fn(DWORD argc, LPTSTR *argv)
{
    const char *ignored;

    /* args and service names live in the same pool */
    mpm_service_set_name(mpm_new_argv->pool, &ignored, argv[0]);

    memset(&globdat.ssStatus, 0, sizeof(globdat.ssStatus));
    globdat.ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    globdat.ssStatus.dwCurrentState = SERVICE_START_PENDING;
    globdat.ssStatus.dwCheckPoint = 1;

    /* ###: utf-ize */
    if (!(globdat.hServiceStatus = RegisterServiceCtrlHandler(argv[0], service_nt_ctrl)))
    {
        ap_log_error(APLOG_MARK, APLOG_ERR | APLOG_STARTUP, apr_get_os_error(),
                     NULL, "Failure registering service handler");
        return;
    }

    /* Report status, no errors, and buy 3 more seconds */
    ReportStatusToSCMgr(SERVICE_START_PENDING, NO_ERROR, 30000);

    /* We need to append all the command arguments passed via StartService()
     * to our running service... which just got here via the SCM...
     * but we hvae no interest in argv[0] for the mpm_new_argv list.
     */
    if (argc > 1)
    {
        char **cmb_data;

        mpm_new_argv->nalloc = mpm_new_argv->nelts + argc - 1;
