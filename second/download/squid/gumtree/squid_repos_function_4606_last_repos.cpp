void
Ftp::Server::AcceptCtrlConnection(const CommAcceptCbParams &params)
{
    MasterXaction::Pointer xact = params.xaction;
    AnyP::PortCfgPointer s = xact->squidPort;

    // NP: it is possible the port was reconfigured when the call or accept() was queued.

    if (params.flag != Comm::OK) {
        // Its possible the call was still queued when the client disconnected
        debugs(33, 2, s->listenConn << ": FTP accept failure: " << xstrerr(params.xerrno));
        return;
    }

    debugs(33, 4, params.conn << ": accepted");
    fd_note(params.conn->fd, "client ftp connect");

    if (s->tcp_keepalive.enabled)
        commSetTcpKeepalive(params.conn->fd, s->tcp_keepalive.idle, s->tcp_keepalive.interval, s->tcp_keepalive.timeout);

    ++incoming_sockets_accepted;

    AsyncJob::Start(new Server(xact));
}