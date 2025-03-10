static void
parse_acl_tos(acl_tos ** head)
{
    acl_tos *l;
    acl_tos **tail = head;  /* sane name below */
    unsigned int tos;           /* Initially uint for strtoui. Casted to tos_t before return */
    char *token = ConfigParser::NextToken();

    if (!token) {
        self_destruct();
        return;
    }

    if (!xstrtoui(token, NULL, &tos, 0, std::numeric_limits<tos_t>::max())) {
        self_destruct();
        return;
    }

    const unsigned int chTos = tos & 0xFC;
    if (chTos != tos) {
        debugs(3, DBG_PARSE_NOTE(DBG_IMPORTANT), "WARNING: Tos value '" << tos << "' adjusted to '" << chTos << "'");
        tos = chTos;
    }

    CBDATA_INIT_TYPE_FREECB(acl_tos, freed_acl_tos);

    l = cbdataAlloc(acl_tos);

    l->tos = (tos_t)tos;

    aclParseAclList(LegacyParser, &l->aclList, token);

    while (*tail)
        tail = &(*tail)->next;

    *tail = l;
}