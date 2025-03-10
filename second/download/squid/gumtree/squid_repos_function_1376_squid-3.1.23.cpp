pid_t
ipcCreate(int type, const char *prog, const char *const args[], const char *name, IpAddress &local_addr, int *rfd, int *wfd, void **hIpc)
{
    unsigned long thread;

    struct ipc_params params;
    int opt;
    int optlen = sizeof(opt);
    DWORD ecode = 0;
    pid_t pid;

    IpAddress tmp_addr;
    struct addrinfo *aiCS = NULL;
    struct addrinfo *aiPS = NULL;

    int crfd = -1;
    int prfd = -1;
    int cwfd = -1;
    int pwfd = -1;
    int x;

    requirePathnameExists(name, prog);

    if (rfd)
        *rfd = -1;

    if (wfd)
        *wfd = -1;

    if (hIpc)
        *hIpc = NULL;

    if (WIN32_OS_version != _WIN_OS_WINNT) {
        getsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char *) &opt, &optlen);
        opt = opt & ~(SO_SYNCHRONOUS_NONALERT | SO_SYNCHRONOUS_ALERT);
        setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char *) &opt, sizeof(opt));
    }

    if (type == IPC_TCP_SOCKET) {
        crfd = cwfd = comm_open(SOCK_STREAM,
                                IPPROTO_TCP,
                                local_addr,
                                COMM_NOCLOEXEC,
                                name);
        prfd = pwfd = comm_open(SOCK_STREAM,
                                IPPROTO_TCP,	/* protocol */
                                local_addr,
                                0,			/* blocking */
                                name);
    } else if (type == IPC_UDP_SOCKET) {
        crfd = cwfd = comm_open(SOCK_DGRAM,
                                IPPROTO_UDP,
                                local_addr,
                                COMM_NOCLOEXEC,
                                name);
        prfd = pwfd = comm_open(SOCK_DGRAM,
                                IPPROTO_UDP,
                                local_addr,
                                0,
                                name);
    } else if (type == IPC_FIFO) {
        debugs(54, 0, "ipcCreate: " << prog << ": use IPC_TCP_SOCKET instead of IP_FIFO on Windows");
        assert(0);
    } else {
        assert(IPC_NONE);
    }

    debugs(54, 3, "ipcCreate: prfd FD " << prfd);
    debugs(54, 3, "ipcCreate: pwfd FD " << pwfd);
    debugs(54, 3, "ipcCreate: crfd FD " << crfd);
    debugs(54, 3, "ipcCreate: cwfd FD " << cwfd);

    if (WIN32_OS_version != _WIN_OS_WINNT) {
        getsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char *) &opt, &optlen);
        opt = opt | SO_SYNCHRONOUS_NONALERT;
        setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char *) &opt, optlen);
    }

    if (crfd < 0) {
        debugs(54, 0, "ipcCreate: Failed to create child FD.");
        return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
    }

    if (pwfd < 0) {
        debugs(54, 0, "ipcCreate: Failed to create server FD.");
        return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
    }

// AYJ: these flags should be neutral, but if not IPv6 version needs adding
    if (type == IPC_TCP_SOCKET || type == IPC_UDP_SOCKET) {

        tmp_addr.InitAddrInfo(aiPS);

        if (getsockname(pwfd, aiPS->ai_addr, &(aiPS->ai_addrlen) ) < 0) {
            debugs(54, 0, "ipcCreate: getsockname: " << xstrerror());
            return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
        }

        tmp_addr = *aiPS;

        debugs(54, 3, "ipcCreate: FD " << pwfd << " sockaddr " << tmp_addr );

        tmp_addr.InitAddrInfo(aiCS);

        if (getsockname(crfd, aiCS->ai_addr, &(aiCS->ai_addrlen) ) < 0) {
            debugs(54, 0, "ipcCreate: getsockname: " << xstrerror());
            return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
        }

        tmp_addr.SetEmpty();
        tmp_addr = *aiCS;

        debugs(54, 3, "ipcCreate: FD " << crfd << " sockaddr " << tmp_addr );
    }

    if (type == IPC_TCP_SOCKET) {
        if (listen(crfd, 1) < 0) {
            debugs(54, 1, "ipcCreate: listen FD " << crfd << ": " << xstrerror());
            return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
        }

        debugs(54, 3, "ipcCreate: FD " << crfd << " listening...");
    }

    /* flush or else we get dup data if unbuffered_logs is set */
    logsFlush();

    params.type = type;

    params.crfd = crfd;

    params.cwfd = cwfd;

    params.PS = *aiPS;

    params.local_addr = local_addr;

    params.prog = prog;

    params.args = (char **) args;

    thread = _beginthreadex(NULL, 0, ipc_thread_1, &params, 0, NULL);

    if (thread == 0) {
        debugs(54, 1, "ipcCreate: _beginthread: " << xstrerror());
        return ipcCloseAllFD(prfd, pwfd, crfd, cwfd);
    }

    /* NP: tmp_addr was left with eiether empty or aiCS in IpAddress format */
    if (comm_connect_addr(pwfd, tmp_addr) == COMM_ERROR) {
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    }

    memset(hello_buf, '\0', HELLO_BUF_SZ);
    x = recv(prfd, (void *)hello_buf, HELLO_BUF_SZ - 1, 0);

    if (x < 0) {
        debugs(54, 0, "ipcCreate: PARENT: hello read test failed");
        debugs(54, 0, "--> read: " << xstrerror());
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    } else if (strcmp(hello_buf, hello_string)) {
        debugs(54, 0, "ipcCreate: PARENT: hello read test failed");
        debugs(54, 0, "--> read returned " << x);
        debugs(54, 0, "--> got '" << rfc1738_escape(hello_buf) << "'");
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    }

    x = send(pwfd, (const void *)ok_string, strlen(ok_string), 0);

    if (x < 0) {
        debugs(54, 0, "ipcCreate: PARENT: OK write test failed");
        debugs(54, 0, "--> read: " << xstrerror());
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    }

    memset(hello_buf, '\0', HELLO_BUF_SZ);
    x = recv(prfd, (void *)hello_buf, HELLO_BUF_SZ - 1, 0);

    if (x < 0) {
        debugs(54, 0, "ipcCreate: PARENT: OK read test failed");
        debugs(54, 0, "--> read: " << xstrerror());
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    } else if (!strcmp(hello_buf, err_string)) {
        debugs(54, 0, "ipcCreate: PARENT: OK read test failed");
        debugs(54, 0, "--> read returned " << x);
        debugs(54, 0, "--> got '" << rfc1738_escape(hello_buf) << "'");
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    }

    hello_buf[x] = '\0';
    pid = atol(hello_buf);
    commSetTimeout(prfd, -1, NULL, NULL);
    commSetNonBlocking(prfd);
    commSetNonBlocking(pwfd);
    commSetCloseOnExec(prfd);
    commSetCloseOnExec(pwfd);

    if (rfd)
        *rfd = prfd;

    if (wfd)
        *wfd = pwfd;

    fd_table[prfd].flags.ipc = 1;

    fd_table[pwfd].flags.ipc = 1;

    fd_table[crfd].flags.ipc = 1;

    fd_table[cwfd].flags.ipc = 1;

    if (Config.sleep_after_fork) {
        /* XXX emulation of usleep() */
        DWORD sl;
        sl = Config.sleep_after_fork / 1000;

        if (sl == 0)
            sl = 1;

        Sleep(sl);
    }

    if (GetExitCodeThread((HANDLE) thread, &ecode) && ecode == STILL_ACTIVE) {
        if (hIpc)
            *hIpc = (HANDLE) thread;

        return pid;
    } else {
        CloseHandle((HANDLE) thread);
        return ipcCloseAllFD(prfd, pwfd, -1, -1);
    }
}