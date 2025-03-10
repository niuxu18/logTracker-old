static apr_status_t file_bucket_read(apr_bucket *e, const char **str,
                                     apr_size_t *len, apr_read_type_e block)
{
    apr_bucket_file *a = e->data;
    apr_file_t *f = a->fd;
    apr_bucket *b = NULL;
    char *buf;
    apr_status_t rv;
    apr_size_t filelength = e->length;  /* bytes remaining in file past offset */
    apr_off_t fileoffset = e->start;
#if APR_HAS_THREADS && !APR_HAS_XTHREAD_FILES
    apr_int32_t flags;
#endif

#if APR_HAS_MMAP
    if (file_make_mmap(e, filelength, fileoffset, a->readpool)) {
        return apr_bucket_read(e, str, len, block);
    }
#endif

#if APR_HAS_THREADS && !APR_HAS_XTHREAD_FILES
    if ((flags = apr_file_flags_get(f)) & APR_FOPEN_XTHREAD) {
        /* this file descriptor is shared across multiple threads and
         * this OS doesn't support that natively, so as a workaround
         * we must reopen the file into a->readpool */
        const char *fname;
        apr_file_name_get(&fname, f);

        rv = apr_file_open(&f, fname, (flags & ~APR_FOPEN_XTHREAD), 0, a->readpool);
        if (rv != APR_SUCCESS)
            return rv;

        a->fd = f;
    }
#endif

    *len = (filelength > APR_BUCKET_BUFF_SIZE)
               ? APR_BUCKET_BUFF_SIZE
               : filelength;
    *str = NULL;  /* in case we die prematurely */
    buf = apr_bucket_alloc(*len, e->list);

    /* Handle offset ... */
    rv = apr_file_seek(f, APR_SET, &fileoffset);
    if (rv != APR_SUCCESS) {
        apr_bucket_free(buf);
        return rv;
    }
    rv = apr_file_read(f, buf, len);
    if (rv != APR_SUCCESS && rv != APR_EOF) {
        apr_bucket_free(buf);
        return rv;
    }
    filelength -= *len;
    /*
     * Change the current bucket to refer to what we read,
     * even if we read nothing because we hit EOF.
     */
    apr_bucket_heap_make(e, buf, *len, apr_bucket_free);

    /* If we have more to read from the file, then create another bucket */
    if (filelength > 0 && rv != APR_EOF) {
        /* for efficiency, we can just build a new apr_bucket struct
         * to wrap around the existing file bucket */
        b = apr_bucket_alloc(sizeof(*b), e->list);
        b->start  = fileoffset + (*len);
        b->length = filelength;
        b->data   = a;
        b->type   = &apr_bucket_type_file;
        b->free   = apr_bucket_free;
        b->list   = e->list;
        APR_BUCKET_INSERT_AFTER(e, b);
    }
    else {
        file_bucket_destroy(a);
    }

    *str = buf;
    return rv;
}