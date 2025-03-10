void
ClientRequestContext::hostHeaderIpVerify(const ipcache_addrs* ia, const DnsLookupDetails &dns)
{
    Comm::ConnectionPointer clientConn = http->getConn()->clientConnection;

    // note the DNS details for the transaction stats.
    http->request->recordLookup(dns);

    if (ia != NULL && ia->count > 0) {
        // Is the NAT destination IP in DNS?
        for (int i = 0; i < ia->count; ++i) {
            if (clientConn->local.matchIPAddr(ia->in_addrs[i]) == 0) {
                debugs(85, 3, HERE << "validate IP " << clientConn->local << " possible from Host:");
                http->request->flags.hostVerified = 1;
                http->doCallouts();
                return;
            }
            debugs(85, 3, HERE << "validate IP " << clientConn->local << " non-match from Host: IP " << ia->in_addrs[i]);
        }
    }
    debugs(85, 3, HERE << "FAIL: validate IP " << clientConn->local << " possible from Host:");
    hostHeaderVerifyFailed("local IP", "any domain IP");
}