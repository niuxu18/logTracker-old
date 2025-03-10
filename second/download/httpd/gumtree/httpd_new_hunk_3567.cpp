    if (config->use_strftime) {
        apr_time_t tNow = apr_time_from_sec(tLogStart);
        apr_time_exp_t e;
        apr_size_t rs;

        apr_time_exp_gmt(&e, tNow);
        apr_strftime(newlog.name, &rs, sizeof(newlog.name), config->szLogRoot, &e);
    }
    else {
        if (config->truncate) {
            apr_snprintf(newlog.name, sizeof(newlog.name), "%s", config->szLogRoot);
        }
        else {
            apr_snprintf(newlog.name, sizeof(newlog.name), "%s.%010d", config->szLogRoot,
                         tLogStart);
        }
    }
    apr_pool_create(&newlog.pool, status->pool);
    if (config->verbose) {
        fprintf(stderr, "Opening file %s\n", newlog.name);
    }
    rv = apr_file_open(&newlog.fd, newlog.name, APR_WRITE | APR_CREATE | APR_APPEND
                       | (config->truncate ? APR_TRUNCATE : 0), APR_OS_DEFAULT, newlog.pool);
    if (rv == APR_SUCCESS) {
        /* Handle post-rotate processing. */
        post_rotate(newlog.pool, &newlog, config, status);

        /* Close out old (previously 'current') logfile, if any. */
        if (status->current.fd) {
            close_logfile(config, &status->current);
        }

        /* New log file is now 'current'. */
        status->current = newlog;
    }
    else {
        char error[120];
        apr_size_t nWrite;

        apr_strerror(rv, error, sizeof error);

        /* Uh-oh. Failed to open the new log file. Try to clear
         * the previous log file, note the lost log entries,
         * and keep on truckin'. */
        if (status->current.fd == NULL) {
            fprintf(stderr, "Could not open log file '%s' (%s)\n", newlog.name, error);
            exit(2);
        }

        /* Throw away new state; it isn't going to be used. */
        apr_pool_destroy(newlog.pool);

        /* Try to keep this error message constant length
         * in case it occurs several times. */
        apr_snprintf(status->errbuf, sizeof status->errbuf,
                     "Resetting log file due to error opening "
                     "new log file, %10d messages lost: %-25.25s\n",
                     status->nMessCount, error);
        nWrite = strlen(status->errbuf);

        if (apr_file_trunc(status->current.fd, 0) != APR_SUCCESS) {
            fprintf(stderr, "Error truncating the file %s\n", status->current.name);
            exit(2);
        }
        if (apr_file_write(status->current.fd, status->errbuf, &nWrite) != APR_SUCCESS) {
            fprintf(stderr, "Error writing to the file %s\n", status->current.name);
            exit(2);
        }
    }

    status->nMessCount = 0;
}

/*
 * Get a size or time param from a string.
 * Parameter 'last' indicates, whether the
 * argument is the last commadnline argument.
