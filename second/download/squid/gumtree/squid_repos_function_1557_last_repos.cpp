static void
peerConnectFailedSilent(CachePeer * p)
{
    p->stats.last_connect_failure = squid_curtime;

    if (!p->tcp_up) {
        debugs(15, 2, "TCP connection to " << p->host << "/" << p->http_port <<
               " dead");
        return;
    }

    -- p->tcp_up;

    if (!p->tcp_up) {
        debugs(15, DBG_IMPORTANT, "Detected DEAD " << neighborTypeStr(p) << ": " << p->name);
        p->stats.logged_state = PEER_DEAD;
    }
}