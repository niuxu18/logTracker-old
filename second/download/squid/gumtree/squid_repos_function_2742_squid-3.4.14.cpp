void
TunnelStateData::readServer(char *buf, size_t len, comm_err_t errcode, int xerrno)
{
    debugs(26, 3, HERE << server.conn << ", read " << len << " bytes, err=" << errcode);

    /*
     * Bail out early on COMM_ERR_CLOSING
     * - close handlers will tidy up for us
     */

    if (errcode == COMM_ERR_CLOSING)
        return;

    if (len > 0) {
        server.bytesIn(len);
        kb_incr(&(statCounter.server.all.kbytes_in), len);
        kb_incr(&(statCounter.server.other.kbytes_in), len);
    }

    if (keepGoingAfterRead(len, errcode, xerrno, server, client))
        copy(len, server, client, WriteClientDone);
}