time_t
parse_rfc1123(const char *str)
{
    struct tm *tm;
    time_t t;
    if (NULL == str)
        return -1;
    tm = parse_date(str);
    if (!tm)
        return -1;
    tm->tm_isdst = -1;
#if HAVE_TIMEGM
    t = timegm(tm);
#elif HAVE_TM_TM_GMTOFF
    t = mktime(tm);
    if (t != -1) {
        struct tm *local = localtime(&t);
        t += local->tm_gmtoff;
    }
#else
    /* some systems do not have tm_gmtoff so we fake it */
    t = mktime(tm);
    if (t != -1) {
        time_t dst = 0;
#if !(defined(_TIMEZONE) || defined(_timezone) || _SQUID_AIX_ || _SQUID_WINDOWS_ || _SQUID_SGI_)
        extern long timezone;
#endif
        /*
         * The following assumes a fixed DST offset of 1 hour,
         * which is probably wrong.
         */
        if (tm->tm_isdst > 0)
            dst = -3600;
#if defined(_timezone) || _SQUID_WINDOWS_
        t -= (_timezone + dst);
#else
        t -= (timezone + dst);
#endif
    }
#endif
    return t;
}