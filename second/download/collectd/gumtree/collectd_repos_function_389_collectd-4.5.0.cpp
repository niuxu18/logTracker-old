static int
handle_dns(const char *buf, int len,
	const struct in6_addr *s_addr,
	const struct in6_addr *d_addr)
{
    rfc1035_header_t qh;
    uint16_t us;
    off_t offset;
    char *t;
    int x;

    /* The DNS header is 12 bytes long */
    if (len < 12)
	return 0;

    memcpy(&us, buf + 0, 2);
    qh.id = ntohs(us);

    memcpy(&us, buf + 2, 2);
    us = ntohs(us);
    fprintf (stderr, "Bytes 0, 1: 0x%04hx\n", us);
    qh.qr = (us >> 15) & 0x01;
    qh.opcode = (us >> 11) & 0x0F;
    qh.aa = (us >> 10) & 0x01;
    qh.tc = (us >> 9) & 0x01;
    qh.rd = (us >> 8) & 0x01;
    qh.ra = (us >> 7) & 0x01;
    qh.z  = (us >> 6) & 0x01;
    qh.ad = (us >> 5) & 0x01;
    qh.cd = (us >> 4) & 0x01;
    qh.rcode = us & 0x0F;

    memcpy(&us, buf + 4, 2);
    qh.qdcount = ntohs(us);

    memcpy(&us, buf + 6, 2);
    qh.ancount = ntohs(us);

    memcpy(&us, buf + 8, 2);
    qh.nscount = ntohs(us);

    memcpy(&us, buf + 10, 2);
    qh.arcount = ntohs(us);

    offset = 12;
    memset(qh.qname, '\0', MAX_QNAME_SZ);
    x = rfc1035NameUnpack(buf, len, &offset, qh.qname, MAX_QNAME_SZ);
    if (0 != x)
	return 0;
    if ('\0' == qh.qname[0])
	sstrncpy (qh.qname, ".", sizeof (qh.qname));
    while ((t = strchr(qh.qname, '\n')))
	*t = ' ';
    while ((t = strchr(qh.qname, '\r')))
	*t = ' ';
    for (t = qh.qname; *t; t++)
	*t = tolower(*t);

    memcpy(&us, buf + offset, 2);
    qh.qtype = ntohs(us);
    memcpy(&us, buf + offset + 2, 2);
    qh.qclass = ntohs(us);

    qh.length = (uint16_t) len;

    /* gather stats */
    qtype_counts[qh.qtype]++;
    qclass_counts[qh.qclass]++;
    opcode_counts[qh.opcode]++;

    if (Callback != NULL)
	    Callback (&qh);

    return 1;
}