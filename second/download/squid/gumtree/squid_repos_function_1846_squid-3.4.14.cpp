const char *
neighborTypeStr(const CachePeer * p)
{
    if (p->type == PEER_NONE)
        return "Non-Peer";

    if (p->type == PEER_SIBLING)
        return "Sibling";

    if (p->type == PEER_MULTICAST)
        return "Multicast Group";

    return "Parent";
}