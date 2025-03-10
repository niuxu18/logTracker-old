static void log_error_core(const char *file, int line, int module_index,
                           int level,
                           apr_status_t status, const server_rec *s,
                           const conn_rec *c,
                           const request_rec *r, apr_pool_t *pool,
                           const char *fmt, va_list args)
{
    char errstr[MAX_STRING_LEN];
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
    char scratch[MAX_STRING_LEN];
#endif
    apr_size_t len, errstrlen;
    apr_file_t *logf = NULL;
    const char *referer;
    int level_and_mask = level & APLOG_LEVELMASK;

    if (r && r->connection) {
        c = r->connection;
    }

    if (s == NULL) {
        /*
         * If we are doing stderr logging (startup), don't log messages that are
         * above the default server log level unless it is a startup/shutdown
         * notice
         */
#ifndef DEBUG
        if ((level_and_mask != APLOG_NOTICE)
            && (level_and_mask > ap_default_loglevel)) {
            return;
        }
#endif

        logf = stderr_log;
    }
    else {
        int configured_level = r ? ap_get_request_module_loglevel(r, module_index)        :
                               c ? ap_get_conn_server_module_loglevel(c, s, module_index) :
                                   ap_get_server_module_loglevel(s, module_index);
        if (s->error_log) {
            /*
             * If we are doing normal logging, don't log messages that are
             * above the module's log level unless it is a startup/shutdown notice
             */
            if ((level_and_mask != APLOG_NOTICE)
                && (level_and_mask > configured_level)) {
                return;
            }

            logf = s->error_log;
        }
        else {
            /*
             * If we are doing syslog logging, don't log messages that are
             * above the module's log level (including a startup/shutdown notice)
             */
            if (level_and_mask > configured_level) {
                return;
            }
        }
    }

    if (logf && ((level & APLOG_STARTUP) != APLOG_STARTUP)) {
        int time_len;

        errstr[0] = '[';
        len = 1;
        time_len = MAX_STRING_LEN - len;
        ap_recent_ctime_ex(errstr + len, apr_time_now(),
                           AP_CTIME_OPTION_USEC, &time_len);
        len += time_len -1;
        errstr[len++] = ']';
        errstr[len++] = ' ';
    } else {
        len = 0;
    }

    if ((level & APLOG_STARTUP) != APLOG_STARTUP) {
        len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                            "[%s] ", priorities[level_and_mask].t_name);

        len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                            "[pid %" APR_PID_T_FMT, getpid());
#if APR_HAS_THREADS
        {
            int result;

            if (ap_mpm_query(AP_MPMQ_IS_THREADED, &result) == APR_SUCCESS
                && result != AP_MPMQ_NOT_SUPPORTED) {
                apr_os_thread_t tid = apr_os_thread_current();
                len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                                    ":tid %pT", &tid);
            }
        }
#endif
        errstr[len++] = ']';
        errstr[len++] = ' ';
    }

    if (file && level_and_mask >= APLOG_DEBUG) {
#if defined(_OSD_POSIX) || defined(WIN32) || defined(__MVS__)
        char tmp[256];
        char *e = strrchr(file, '/');
#ifdef WIN32
        if (!e) {
            e = strrchr(file, '\\');
        }
#endif

        /* In OSD/POSIX, the compiler returns for __FILE__
         * a string like: __FILE__="*POSIX(/usr/include/stdio.h)"
         * (it even returns an absolute path for sources in
         * the current directory). Here we try to strip this
         * down to the basename.
         */
        if (e != NULL && e[1] != '\0') {
            apr_snprintf(tmp, sizeof(tmp), "%s", &e[1]);
            e = &tmp[strlen(tmp)-1];
            if (*e == ')') {
                *e = '\0';
            }
            file = tmp;
        }
#else /* _OSD_POSIX || WIN32 */
        const char *p;
        /* On Unix, __FILE__ may be an absolute path in a
         * VPATH build. */
        if (file[0] == '/' && (p = ap_strrchr_c(file, '/')) != NULL) {
            file = p + 1;
        }
#endif /*_OSD_POSIX || WIN32 */
        len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                            "%s(%d): ", file, line);
    }

    if (c) {
        /* XXX: TODO: add a method of selecting whether logged remote
         * addresses are in dotted quad or resolved form... dotted
         * quad is the most secure, which is why I'm implementing it
         * first. -djg
         */
        /*
         * remote_ip can be client or backend server. If we have a scoreboard
         * handle, it is likely a client.
         */
        len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                            c->sbh ? "[client %s:%d] " : "[remote %s:%d] ",
                            c->remote_ip, c->remote_addr->port);
    }
    if (status != 0) {
        if (status < APR_OS_START_EAIERR) {
            len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                                "(%d)", status);
        }
        else if (status < APR_OS_START_SYSERR) {
            len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                                "(EAI %d)", status - APR_OS_START_EAIERR);
        }
        else if (status < 100000 + APR_OS_START_SYSERR) {
            len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                                "(OS %d)", status - APR_OS_START_SYSERR);
        }
        else {
            len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                                "(os 0x%08x)", status - APR_OS_START_SYSERR);
        }
        apr_strerror(status, errstr + len, MAX_STRING_LEN - len);
        len += strlen(errstr + len);
        if (MAX_STRING_LEN - len > 2) {
            errstr[len++] = ':';
            errstr[len++] = ' ';
            errstr[len] = '\0';
        }
    }

    errstrlen = len;
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
    if (apr_vsnprintf(scratch, MAX_STRING_LEN - len, fmt, args)) {
        len += ap_escape_errorlog_item(errstr + len, scratch,
                                       MAX_STRING_LEN - len);
    }
#else
    len += apr_vsnprintf(errstr + len, MAX_STRING_LEN - len, fmt, args);
#endif

    if (   r && (referer = apr_table_get(r->headers_in, "Referer"))
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
        && ap_escape_errorlog_item(scratch, referer, MAX_STRING_LEN - len)
#endif
        ) {
        len += apr_snprintf(errstr + len, MAX_STRING_LEN - len,
                            ", referer: %s",
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
                            scratch
#else
                            referer
#endif
                            );
    }

    /* NULL if we are logging to syslog */
    if (logf) {
        /* Truncate for the terminator (as apr_snprintf does) */
        if (len > MAX_STRING_LEN - sizeof(APR_EOL_STR)) {
            len = MAX_STRING_LEN - sizeof(APR_EOL_STR);
        }
        strcpy(errstr + len, APR_EOL_STR);
        apr_file_puts(errstr, logf);
        apr_file_flush(logf);
    }
#ifdef HAVE_SYSLOG
    else {
        syslog(level_and_mask < LOG_PRIMASK ? level_and_mask : APLOG_DEBUG,
               "%s", errstr);
    }
#endif

    ap_run_error_log(file, line, module_index, level, status, s, r, pool,
                     errstr + errstrlen);
}