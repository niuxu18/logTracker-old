void
TunnelStateData::WriteServerDone(const Comm::ConnectionPointer &, char *buf, size_t len, comm_err_t flag, int xerrno, void *data)
{
    TunnelStateData *tunnelState = (TunnelStateData *)data;
    assert (cbdataReferenceValid (tunnelState));
    tunnelState->server.writer = NULL;

    tunnelState->writeServerDone(buf, len, flag, xerrno);
}