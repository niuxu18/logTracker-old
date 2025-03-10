static apr_status_t apr_file_transfer_contents(const char *from_path,
                                               const char *to_path,
                                               apr_int32_t flags,
                                               apr_fileperms_t to_perms,
                                               apr_pool_t *pool)
{
    apr_file_t *s, *d;
    apr_status_t status;
    apr_fileperms_t perms;

    /* Open source file. */
    status = apr_file_open(&s, from_path, APR_READ | APR_LARGEFILE, 
                           APR_OS_DEFAULT, pool);
    if (status)
        return status;

    /* Maybe get its permissions. */
    if (to_perms == APR_FILE_SOURCE_PERMS) {
#if defined(HAVE_FSTAT64) && defined(O_LARGEFILE) && SIZEOF_OFF_T == 4
        struct stat64 st;

        if (fstat64(s->filedes, &st) != 0)
            return errno;

        perms = apr_unix_mode2perms(st.st_mode);  
#else
        apr_finfo_t finfo;

        status = apr_file_info_get(&finfo, APR_FINFO_PROT, s);
        if (status != APR_SUCCESS && status != APR_INCOMPLETE) {
            apr_file_close(s);  /* toss any error */
            return status;
        }
        perms = finfo.protection;
#endif
    }
    else
        perms = to_perms;

    /* Open dest file. */
    status = apr_file_open(&d, to_path, flags, perms, pool);
    if (status) {
        apr_file_close(s);  /* toss any error */
        return status;
    }

#if BUFSIZ > APR_FILE_DEFAULT_BUFSIZE
#define COPY_BUFSIZ BUFSIZ
#else
#define COPY_BUFSIZ APR_FILE_DEFAULT_BUFSIZE
#endif

    /* Copy bytes till the cows come home. */
    while (1) {
        char buf[COPY_BUFSIZ];
        apr_size_t bytes_this_time = sizeof(buf);
        apr_status_t read_err;
        apr_status_t write_err;

        /* Read 'em. */
        read_err = apr_file_read(s, buf, &bytes_this_time);
        if (read_err && !APR_STATUS_IS_EOF(read_err)) {
            apr_file_close(s);  /* toss any error */
            apr_file_close(d);  /* toss any error */
            return read_err;
        }

        /* Write 'em. */
        write_err = apr_file_write_full(d, buf, bytes_this_time, NULL);
        if (write_err) {
            apr_file_close(s);  /* toss any error */
            apr_file_close(d);  /* toss any error */
            return write_err;
        }

        if (read_err && APR_STATUS_IS_EOF(read_err)) {
            status = apr_file_close(s);
            if (status) {
                apr_file_close(d);  /* toss any error */
                return status;
            }

            /* return the results of this close: an error, or success */
            return apr_file_close(d);
        }
    }
    /* NOTREACHED */
}