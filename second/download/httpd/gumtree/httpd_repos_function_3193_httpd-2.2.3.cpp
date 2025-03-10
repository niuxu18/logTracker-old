static apr_status_t file_dup(apr_file_t **new_file, 
                             apr_file_t *old_file, apr_pool_t *p,
                             int which_dup)
{
    int rv;
    
    if (which_dup == 2) {
        if ((*new_file) == NULL) {
            /* We can't dup2 unless we have a valid new_file */
            return APR_EINVAL;
        }
        rv = dup2(old_file->filedes, (*new_file)->filedes);
    } else {
        rv = dup(old_file->filedes);
    }

    if (rv == -1)
        return errno;
    
    if (which_dup == 1) {
        (*new_file) = (apr_file_t *)apr_pcalloc(p, sizeof(apr_file_t));
        (*new_file)->pool = p;
        (*new_file)->filedes = rv;
    }

    (*new_file)->fname = apr_pstrdup(p, old_file->fname);
    (*new_file)->buffered = old_file->buffered;

    /* If the existing socket in a dup2 is already buffered, we
     * have an existing and valid (hopefully) mutex, so we don't
     * want to create it again as we could leak!
     */
#if APR_HAS_THREADS
    if ((*new_file)->buffered && !(*new_file)->thlock && old_file->thlock) {
        apr_thread_mutex_create(&((*new_file)->thlock),
                                APR_THREAD_MUTEX_DEFAULT, p);
    }
#endif
    /* As above, only create the buffer if we haven't already
     * got one.
     */
    if ((*new_file)->buffered && !(*new_file)->buffer) {
        (*new_file)->buffer = apr_palloc(p, APR_FILE_BUFSIZE);
    }

    /* this is the way dup() works */
    (*new_file)->blocking = old_file->blocking; 

    /* make sure unget behavior is consistent */
    (*new_file)->ungetchar = old_file->ungetchar;

    /* apr_file_dup2() retains the original cleanup, reflecting 
     * the existing inherit and nocleanup flags.  This means, 
     * that apr_file_dup2() cannot be called against an apr_file_t
     * already closed with apr_file_close, because the expected
     * cleanup was already killed.
     */
    if (which_dup == 2) {
        return APR_SUCCESS;
    }

    /* apr_file_dup() retains all old_file flags with the exceptions
     * of APR_INHERIT and APR_FILE_NOCLEANUP.
     * The user must call apr_file_inherit_set() on the dupped 
     * apr_file_t when desired.
     */
    (*new_file)->flags = old_file->flags
                       & ~(APR_INHERIT | APR_FILE_NOCLEANUP);

    apr_pool_cleanup_register((*new_file)->pool, (void *)(*new_file),
                              apr_unix_file_cleanup, 
                              apr_unix_file_cleanup);
#ifndef WAITIO_USES_POLL
    /* Start out with no pollset.  apr_wait_for_io_or_timeout() will
     * initialize the pollset if needed.
     */
    (*new_file)->pollset = NULL;
#endif
    return APR_SUCCESS;
}