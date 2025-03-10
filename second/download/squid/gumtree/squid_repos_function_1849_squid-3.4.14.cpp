bool
peerAllowedToUse(const CachePeer * p, HttpRequest * request)
{

    const CachePeerDomainList *d = NULL;
    assert(request != NULL);

    if (neighborType(p, request) == PEER_SIBLING) {
#if PEER_MULTICAST_SIBLINGS
        if (p->type == PEER_MULTICAST && p->options.mcast_siblings &&
                (request->flags.noCache || request->flags.refresh || request->flags.loopDetected || request->flags.needValidation))
            debugs(15, 2, "peerAllowedToUse(" << p->name << ", " << request->GetHost() << ") : multicast-siblings optimization match");
#endif
        if (request->flags.noCache)
            return false;

        if (request->flags.refresh)
            return false;

        if (request->flags.loopDetected)
            return false;

        if (request->flags.needValidation)
            return false;
    }

    // CONNECT requests are proxy requests. Not to be forwarded to origin servers.
    // Unless the destination port matches, in which case we MAY perform a 'DIRECT' to this CachePeer.
    if (p->options.originserver && request->method == Http::METHOD_CONNECT && request->port != p->in_addr.port())
        return false;

    if (p->peer_domain == NULL && p->access == NULL)
        return true;

    bool do_ping = false;
    for (d = p->peer_domain; d; d = d->next) {
        if (0 == matchDomainName(request->GetHost(), d->domain)) {
            do_ping = d->do_ping;
            break;
        }

        do_ping = !d->do_ping;
    }

    if (p->peer_domain && !do_ping)
        return false;

    if (p->access == NULL)
        return do_ping;

    ACLFilledChecklist checklist(p->access, request, NULL);

    return (checklist.fastCheck() == ACCESS_ALLOWED);
}