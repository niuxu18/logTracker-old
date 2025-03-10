int
cbdataReferenceValid(const void *p)
{
    cbdata *c;

    if (p == NULL)
        return 1;       /* A NULL pointer cannot become invalid */

    debugs(45, 9, p);

#if HASHED_CBDATA
    c = (cbdata *) hash_lookup(cbdata_htable, p);
#else
    c = (cbdata *) (((char *) p) - cbdata::Offset);
#endif

    c->check(__LINE__);

    assert(c->locks > 0);

    return c->valid;
}