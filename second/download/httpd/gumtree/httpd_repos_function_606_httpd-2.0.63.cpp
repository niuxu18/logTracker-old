X509_STORE *SSL_X509_STORE_create(char *cpFile, char *cpPath)
{
    X509_STORE *pStore;
    X509_LOOKUP *pLookup;

    if (cpFile == NULL && cpPath == NULL)
        return NULL;
    if ((pStore = X509_STORE_new()) == NULL)
        return NULL;
    if (cpFile != NULL) {
        pLookup = X509_STORE_add_lookup(pStore, X509_LOOKUP_file());
        if (pLookup == NULL) {
            X509_STORE_free(pStore);
            return NULL;
        }
        X509_LOOKUP_load_file(pLookup, cpFile, X509_FILETYPE_PEM);
    }
    if (cpPath != NULL) {
        pLookup = X509_STORE_add_lookup(pStore, X509_LOOKUP_hash_dir());
        if (pLookup == NULL) {
            X509_STORE_free(pStore);
            return NULL;
        }
        X509_LOOKUP_add_dir(pLookup, cpPath, X509_FILETYPE_PEM);
    }
    return pStore;
}