static void
parse_hostdomaintype(void)
{
    char *host = NULL;
    char *type = NULL;
    char *domain = NULL;

    if (!(host = ConfigParser::NextToken()))
        self_destruct();

    if (!(type = ConfigParser::NextToken()))
        self_destruct();

    while ((domain = ConfigParser::NextToken())) {
        NeighborTypeDomainList *l = NULL;
        NeighborTypeDomainList **L = NULL;
        CachePeer *p;

        if ((p = peerFindByName(host)) == NULL) {
            debugs(15, DBG_CRITICAL, "" << cfg_filename << ", line " << config_lineno << ": No cache_peer '" << host << "'");
            return;
        }

        l = static_cast<NeighborTypeDomainList *>(xcalloc(1, sizeof(NeighborTypeDomainList)));
        l->type = parseNeighborType(type);
        l->domain = xstrdup(domain);

        for (L = &(p->typelist); *L; L = &((*L)->next));
        *L = l;
    }
}