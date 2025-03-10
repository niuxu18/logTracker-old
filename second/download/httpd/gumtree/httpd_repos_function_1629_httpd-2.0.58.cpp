void *util_ldap_dn_compare_node_copy(util_ald_cache_t *cache, void *c)
{
    util_dn_compare_node_t *n = (util_dn_compare_node_t *)c;
    util_dn_compare_node_t *node = (util_dn_compare_node_t *)util_ald_alloc(cache, sizeof(util_dn_compare_node_t));
    if (node) {
        if (!(node->reqdn = util_ald_strdup(cache, n->reqdn)) ||
            !(node->dn = util_ald_strdup(cache, n->dn))) {
            util_ldap_dn_compare_node_free(cache, node);
            return NULL;
        }
        return node;
    }
    else {
        return NULL;
    }
}