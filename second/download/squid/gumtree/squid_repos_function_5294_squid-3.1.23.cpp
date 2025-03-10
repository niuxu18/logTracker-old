void
IcmpSquid::Recv()
{
    int n;
    static int fail_count = 0;
    pingerReplyData preply;
    static IpAddress F;

    commSetSelect(icmp_sock, COMM_SELECT_READ, icmpSquidRecv, NULL, 0);
    memset(&preply, '\0', sizeof(pingerReplyData));
    n = comm_udp_recv(icmp_sock,
                      (char *) &preply,
                      sizeof(pingerReplyData),
                      0);

    if (n < 0 && EAGAIN != errno) {
        debugs(37, 1, HERE << "recv: " << xstrerror());

        if (errno == ECONNREFUSED)
            Close();

        if (errno == ECONNRESET)
            Close();

        if (++fail_count == 10)
            Close();

        return;
    }

    fail_count = 0;

    /** If its a test probe from the pinger. Do nothing. */
    if (n == 0) {
        return;
    }

    F = preply.from;

    F.SetPort(0);

    switch (preply.opcode) {

    case S_ICMP_ECHO:
        debugs(37,4, HERE << " ICMP_ECHO of " << preply.from << " gave: hops=" << preply.hops <<", rtt=" << preply.rtt);
        break;

    case S_ICMP_DOM:
        debugs(37,4, HERE << " DomainPing of " << preply.from << " gave: hops=" << preply.hops <<", rtt=" << preply.rtt);
        netdbHandlePingReply(F, preply.hops, preply.rtt);
        break;

    default:
        debugs(37, 1, HERE << "Bad opcode: " << preply.opcode << " from " << F);
        break;
    }
}