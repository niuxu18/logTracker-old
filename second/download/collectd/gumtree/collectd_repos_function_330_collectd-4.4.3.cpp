static inline int ignore_list_match (const struct in6_addr *addr)
{
    ip_list_t *ptr;

    for (ptr = IgnoreList; ptr != NULL; ptr = ptr->next)
	if (cmp_in6_addr (addr, &ptr->addr) == 0)
	    return (1);
    return (0);
}