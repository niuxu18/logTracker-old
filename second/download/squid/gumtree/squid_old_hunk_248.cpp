SQUIDCEXTERN void leakInit(void);
SQUIDCEXTERN void *leakAddFL(void *, const char *, int);
SQUIDCEXTERN void *leakTouchFL(void *, const char *, int);
SQUIDCEXTERN void *leakFreeFL(void *, const char *, int);
#endif

/* logfile.c */
SQUIDCEXTERN Logfile *logfileOpen(const char *path, size_t bufsz, int);
SQUIDCEXTERN void logfileClose(Logfile * lf);
SQUIDCEXTERN void logfileRotate(Logfile * lf);
SQUIDCEXTERN void logfileWrite(Logfile * lf, void *buf, size_t len);
SQUIDCEXTERN void logfileFlush(Logfile * lf);
SQUIDCEXTERN void logfilePrintf(Logfile * lf, const char *fmt,...) PRINTF_FORMAT_ARG2;

/*
 * prototypes for system functions missing from system includes
 */

#ifdef _SQUID_SOLARIS_

SQUIDCEXTERN int getrusage(int, struct rusage *);
SQUIDCEXTERN int getpagesize(void);
#if !defined(_XPG4_2) && !(defined(__EXTENSIONS__) || \
(!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)))
SQUIDCEXTERN int gethostname(char *, int);
#endif
#endif

/*
 * hack to allow snmp access to the statistics counters
 */
SQUIDCEXTERN StatCounters *snmpStatGet(int);

/* Vary support functions */
SQUIDCEXTERN int varyEvaluateMatch(StoreEntry * entry, HttpRequest * req);

/* CygWin & Windows NT Port */
/* win32.c */
#ifdef _SQUID_WIN32_
SQUIDCEXTERN int WIN32_Subsystem_Init(int *, char ***);
SQUIDCEXTERN void WIN32_sendSignal(int);
SQUIDCEXTERN void WIN32_Abort(int);
SQUIDCEXTERN void WIN32_Exit(void);
SQUIDCEXTERN void WIN32_SetServiceCommandLine(void);
SQUIDCEXTERN void WIN32_InstallService(void);
SQUIDCEXTERN void WIN32_RemoveService(void);
SQUIDCEXTERN int SquidMain(int, char **);
#endif /* _SQUID_WIN32_ */
#ifdef _SQUID_MSWIN_

SQUIDCEXTERN int WIN32_pipe(int[2]);

SQUIDCEXTERN int WIN32_getrusage(int, struct rusage *);
SQUIDCEXTERN void WIN32_ExceptionHandlerInit(void);

SQUIDCEXTERN int Win32__WSAFDIsSet(int fd, fd_set* set);
SQUIDCEXTERN DWORD WIN32_IpAddrChangeMonitorInit();

#endif

/* external_acl.c */
class external_acl;
        SQUIDCEXTERN void parse_externalAclHelper(external_acl **);

        SQUIDCEXTERN void dump_externalAclHelper(StoreEntry * sentry, const char *name, const external_acl *);

        SQUIDCEXTERN void free_externalAclHelper(external_acl **);

        typedef void EAH(void *data, void *result);
        class ACLChecklist;
            SQUIDCEXTERN void externalAclLookup(ACLChecklist * ch, void *acl_data, EAH * handler, void *data);

            SQUIDCEXTERN void externalAclInit(void);

            SQUIDCEXTERN void externalAclShutdown(void);

            SQUIDCEXTERN char *strtokFile(void);

#if USE_WCCPv2

            SQUIDCEXTERN void parse_wccp2_method(int *v);
            SQUIDCEXTERN void free_wccp2_method(int *v);
            SQUIDCEXTERN void dump_wccp2_method(StoreEntry * e, const char *label, int v);
            SQUIDCEXTERN void parse_wccp2_amethod(int *v);
            SQUIDCEXTERN void free_wccp2_amethod(int *v);
            SQUIDCEXTERN void dump_wccp2_amethod(StoreEntry * e, const char *label, int v);

            SQUIDCEXTERN void parse_wccp2_service(void *v);
            SQUIDCEXTERN void free_wccp2_service(void *v);
            SQUIDCEXTERN void dump_wccp2_service(StoreEntry * e, const char *label, void *v);

            SQUIDCEXTERN int check_null_wccp2_service(void *v);

            SQUIDCEXTERN void parse_wccp2_service_info(void *v);

            SQUIDCEXTERN void free_wccp2_service_info(void *v);

            SQUIDCEXTERN void dump_wccp2_service_info(StoreEntry * e, const char *label, void *v);

#endif

#endif /* SQUID_PROTOS_H */
