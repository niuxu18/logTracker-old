static unsigned long index_name_hash_LHASH_HASH(const void *a) {
        return index_name_hash((const char **)a);
    }