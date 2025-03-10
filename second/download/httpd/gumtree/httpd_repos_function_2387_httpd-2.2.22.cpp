apr_status_t mpm_service_to_start(const char **display_name, apr_pool_t *p)
{
    HANDLE hProc = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    HANDLE waitfor[2];

    /* Prevent holding open the (hidden) console */
    ap_real_exit_code = 0;

     /* GetCurrentThread returns a psuedo-handle, we need
      * a real handle for another thread to wait upon.
      */
    if (!DuplicateHandle(hProc, hThread, hProc, &(globdat.mpm_thread),
                         0, FALSE, DUPLICATE_SAME_ACCESS)) {
        return APR_ENOTHREAD;
    }

    if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        globdat.service_init = CreateEvent(NULL, FALSE, FALSE, NULL);
        globdat.service_term = CreateMutex(NULL, TRUE, NULL);
        if (!globdat.service_init || !globdat.service_term) {
             return APR_EGENERAL;
        }

        globdat.service_thread = CreateThread(NULL, 0, service_nt_dispatch_thread,
                                              NULL, 0, &globdat.service_thread_id);
    }
    else /* osver.dwPlatformId != VER_PLATFORM_WIN32_NT */
    {
        if (!RegisterServiceProcess(0, 1))
            return GetLastError();

        globdat.service_init = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!globdat.service_init) {
            return APR_EGENERAL;
        }

        globdat.service_thread = CreateThread(NULL, 0, monitor_service_9x_thread,
                                              (LPVOID) mpm_service_name, 0,
                                              &globdat.service_thread_id);
    }

    if (!globdat.service_thread) {
        return APR_ENOTHREAD;
    }

    waitfor[0] = globdat.service_init;
    waitfor[1] = globdat.service_thread;

    /* Wait for controlling thread init or termination */
    if (WaitForMultipleObjects(2, waitfor, FALSE, 10000) != WAIT_OBJECT_0) {
        return APR_ENOTHREAD;
    }

    atexit(service_stopped);
    *display_name = mpm_display_name;
    return APR_SUCCESS;
}