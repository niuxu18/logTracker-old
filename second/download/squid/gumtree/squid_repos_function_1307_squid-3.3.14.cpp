static void
peerAddFwdServer(FwdServer ** FSVR, CachePeer * p, hier_code code)
{
    FwdServer *fs = (FwdServer *)memAllocate(MEM_FWD_SERVER);
    debugs(44, 5, "peerAddFwdServer: adding " <<
           (p ? p->host : "DIRECT")  << " " <<
           hier_code_str[code]  );
    fs->_peer = cbdataReference(p);
    fs->code = code;

    while (*FSVR)
        FSVR = &(*FSVR)->next;

    *FSVR = fs;
}