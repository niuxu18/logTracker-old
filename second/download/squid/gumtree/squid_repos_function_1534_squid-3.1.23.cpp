unsigned int
ClassCNetPool::makeKey (IpAddress &src_addr) const
{
    /* IPv4 required for this pool */
    if ( !src_addr.IsIPv4() )
        return 1;

    struct in_addr net;
    src_addr.GetInAddr(net);
    return ( (ntohl(net.s_addr) >> 8) & 0xff);
}