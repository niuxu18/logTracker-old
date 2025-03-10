static void
free_external_acl(void *data)
{
    external_acl *p = static_cast<external_acl *>(data);
    safe_free(p->name);

    while (p->format) {
        external_acl_format *f = p->format;
        p->format = f->next;
        cbdataFree(f);
    }

    wordlistDestroy(&p->cmdline);

    if (p->theHelper) {
        helperShutdown(p->theHelper);
        helperFree(p->theHelper);
        p->theHelper = NULL;
    }

    while (p->lru_list.tail)
        external_acl_cache_delete(p, static_cast<external_acl_entry *>(p->lru_list.tail->data));
    if (p->cache)
        hashFreeMemory(p->cache);
}