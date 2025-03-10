    return process;
}

static void usage(process_rec *process)
{
    const char *bin = process->argv[0];
    char pad[MAX_STRING_LEN];
    unsigned i;

    for (i = 0; i < strlen(bin); i++) {
        pad[i] = ' ';
    }

    pad[i] = '\0';

    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "Usage: %s [-D name] [-d directory] [-f file]", bin);

    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "       %s [-C \"directive\"] [-c \"directive\"]", pad);

#ifdef WIN32
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "       %s [-w] [-k start|restart|stop|shutdown] [-n service_name]", pad);
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "       %s [-k install|config|uninstall] [-n service_name]",
                 pad);
#else
/* XXX not all MPMs support signalling the server in general or graceful-stop
 * in particular
 */
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "       %s [-k start|restart|graceful|graceful-stop|stop]",
                 pad);
#endif
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "       %s [-v] [-V] [-h] [-l] [-L] [-t] [-T] [-S]", pad);
    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "Options:");

    ap_log_error(APLOG_MARK, APLOG_STARTUP, 0, NULL,
                 "  -D name            : define a name for use in "
                 "<IfDefine name> directives");
