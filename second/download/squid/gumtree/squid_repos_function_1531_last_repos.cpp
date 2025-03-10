void
peerAlive(CachePeer *p)
{
    if (p->stats.logged_state == PEER_DEAD && p->tcp_up) {
        debugs(15, DBG_IMPORTANT, "Detected REVIVED " << neighborTypeStr(p) << ": " << p->name);
        p->stats.logged_state = PEER_ALIVE;
        peerClearRR();
        if (p->standby.mgr.valid())
            PeerPoolMgr::Checkpoint(p->standby.mgr, "revived peer");
    }

    p->stats.last_reply = squid_curtime;
    p->stats.probe_start = 0;
}