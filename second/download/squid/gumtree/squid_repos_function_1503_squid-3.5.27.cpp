static void
icpLogIcp(const Ip::Address &caddr, LogTags logcode, int len, const char *url, int delay)
{
    AccessLogEntry::Pointer al = new AccessLogEntry();

    if (LOG_TAG_NONE == logcode)
        return;

    if (LOG_ICP_QUERY == logcode)
        return;

    clientdbUpdate(caddr, logcode, AnyP::PROTO_ICP, len);

    if (!Config.onoff.log_udp)
        return;

    al->icp.opcode = ICP_QUERY;

    al->url = url;

    al->cache.caddr = caddr;

    // XXX: move to use icp.clientReply instead
    al->http.clientReplySz.payloadData = len;

    al->cache.code = logcode;

    al->cache.msec = delay;

    accessLogLog(al, NULL);
}