static apr_status_t proc_mutex_proc_pthread_create(apr_proc_mutex_t *new_mutex,
                                                   const char *fname)
{
    apr_status_t rv;
    int fd;
    pthread_mutexattr_t mattr;

    fd = open("/dev/zero", O_RDWR);
    if (fd < 0) {
        return errno;
    }

    new_mutex->pthread_interproc = (pthread_mutex_t *)mmap(
                                       (caddr_t) 0, 
                                       sizeof(pthread_mutex_t), 
                                       PROT_READ | PROT_WRITE, MAP_SHARED,
                                       fd, 0); 
    if (new_mutex->pthread_interproc == (pthread_mutex_t *) (caddr_t) -1) {
        return errno;
    }
    close(fd);
    if ((rv = pthread_mutexattr_init(&mattr))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }
    if ((rv = pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }

#ifdef HAVE_PTHREAD_MUTEXATTR_SETROBUST_NP
    if ((rv = pthread_mutexattr_setrobust_np(&mattr, 
                                               PTHREAD_MUTEX_ROBUST_NP))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }
    if ((rv = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }
#endif

    if ((rv = pthread_mutex_init(new_mutex->pthread_interproc, &mattr))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }

    if ((rv = pthread_mutexattr_destroy(&mattr))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }

    new_mutex->curr_locked = 0;
    apr_pool_cleanup_register(new_mutex->pool,
                              (void *)new_mutex,
                              proc_mutex_proc_pthread_cleanup, 
                              apr_pool_cleanup_null);
    return APR_SUCCESS;
}