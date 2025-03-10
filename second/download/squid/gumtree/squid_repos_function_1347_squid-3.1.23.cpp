static void
icpLogIcp(const IpAddress &caddr, log_type logcode, int len, const char *url, int delay)
{
    AccessLogEntry al;

    if (LOG_TAG_NONE == logcode)
        return;

    if (LOG_ICP_QUERY == logcode)
        return;

    clientdbUpdate(caddr, logcode, PROTO_ICP, len);

    if (!Config.onoff.log_udp)
        return;

    al.icp.opcode = ICP_QUERY;

    al.url = url;

    al.cache.caddr = caddr;

    al.cache.replySize = len;

    al.cache.code = logcode;

    al.cache.msec = delay;

    accessLogLog(&al, NULL);
}