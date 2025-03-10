static apr_int32_t get_offset(struct tm *tm)
{
#if defined(HAVE_STRUCT_TM_TM_GMTOFF)
    return tm->tm_gmtoff;
#elif defined(HAVE_STRUCT_TM___TM_GMTOFF)
    return tm->__tm_gmtoff;
#else
#ifdef NETWARE
    /* Need to adjust the global variable each time otherwise
        the web server would have to be restarted when daylight
        savings changes.
    */
    if (daylightOnOff) {
        return server_gmt_offset + daylightOffset;
    }
#else
    if (tm->tm_isdst)
        return server_gmt_offset + 3600;
#endif
    return server_gmt_offset;
#endif
}