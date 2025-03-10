Ip::Address
networkFromInaddr(const Ip::Address &in)
{
    Ip::Address out;

    out = in;

    /* in IPv6 the 'network' should be the routing section. */
    if ( in.isIPv6() ) {
        out.applyMask(64, AF_INET6);
        debugs(14, 5, "networkFromInaddr : Masked IPv6 Address to " << in << "/64 routing part.");
        return out;
    }

#if USE_CLASSFUL
    struct in_addr b;

    in.getInAddr(b);

    if (IN_CLASSC(b.s_addr))
        b.s_addr &= IN_CLASSC_NET;
    else if (IN_CLASSB(b.s_addr))
        b.s_addr &= IN_CLASSB_NET;
    else if (IN_CLASSA(b.s_addr))
        b.s_addr &= IN_CLASSA_NET;

    out = b;

#endif

    debugs(14, 5, "networkFromInaddr : Masked IPv4 Address to " << out << "/24.");

    /* use /24 for everything under IPv4 */
    out.applyMask(24, AF_INET);
    debugs(14, 5, "networkFromInaddr : Masked IPv4 Address to " << in << "/24.");

    return out;
}