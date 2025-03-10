static dav_error * dav_generic_save_lock_record(dav_lockdb *lockdb,
                                                apr_datum_t key,
                                                dav_lock_discovery *direct,
                                                dav_lock_indirect *indirect)
{
    dav_error *err;
    apr_status_t status;
    apr_datum_t val = { 0 };
    char *ptr;
    dav_lock_discovery *dp = direct;
    dav_lock_indirect *ip = indirect;

#if DAV_DEBUG
    if (lockdb->ro) {
        return dav_new_error(lockdb->info->pool,
                             HTTP_INTERNAL_SERVER_ERROR, 0,
                             "INTERNAL DESIGN ERROR: the lockdb was opened "
                             "readonly, but an attempt to save locks was "
                             "performed.");
    }
#endif

    if ((err = dav_generic_really_open_lockdb(lockdb)) != NULL) {
        /* ### add a higher-level error? */
        return err;
    }

    /* If nothing to save, delete key */
    if (dp == NULL && ip == NULL) {
        /* don't fail if the key is not present */
        /* ### but what about other errors? */
        apr_dbm_delete(lockdb->info->db, key);
        return NULL;
    }

    while(dp) {
        val.dsize += dav_size_direct(dp);
        dp = dp->next;
    }
    while(ip) {
        val.dsize += dav_size_indirect(ip);
        ip = ip->next;
    }

    /* ### can this be apr_palloc() ? */
    /* ### hmmm.... investigate the use of a buffer here */
    ptr = val.dptr = apr_pcalloc(lockdb->info->pool, val.dsize);
    dp  = direct;
    ip  = indirect;

    while(dp) {
        /* Direct lock - lock_discovery struct follows */
        *ptr++ = DAV_LOCK_DIRECT;
        memcpy(ptr, dp, sizeof(dp->f));        /* Fixed portion of struct */
        ptr += sizeof(dp->f);
        memcpy(ptr, dp->locktoken, sizeof(*dp->locktoken));
        ptr += sizeof(*dp->locktoken);
        if (dp->owner == NULL) {
            *ptr++ = '\0';
        }
        else {
            memcpy(ptr, dp->owner, strlen(dp->owner) + 1);
            ptr += strlen(dp->owner) + 1;
        }
        if (dp->auth_user == NULL) {
            *ptr++ = '\0';
        }
        else {
            memcpy(ptr, dp->auth_user, strlen(dp->auth_user) + 1);
            ptr += strlen(dp->auth_user) + 1;
        }

        dp = dp->next;
    }

    while(ip) {
        /* Indirect lock prefix */
        *ptr++ = DAV_LOCK_INDIRECT;

        memcpy(ptr, ip->locktoken, sizeof(*ip->locktoken));
        ptr += sizeof(*ip->locktoken);

        memcpy(ptr, &ip->timeout, sizeof(ip->timeout));
        ptr += sizeof(ip->timeout);

        memcpy(ptr, &ip->key.dsize, sizeof(ip->key.dsize));
        ptr += sizeof(ip->key.dsize);

        memcpy(ptr, ip->key.dptr, ip->key.dsize);
        ptr += ip->key.dsize;

        ip = ip->next;
    }

    if ((status = apr_dbm_store(lockdb->info->db, key, val)) != APR_SUCCESS) {
        /* ### more details? add an error_id? */
        err = dav_generic_dbm_new_error(lockdb->info->db, lockdb->info->pool,
                                        status);
        return dav_push_error(lockdb->info->pool,
                              HTTP_INTERNAL_SERVER_ERROR,
                              DAV_ERR_LOCK_SAVE_LOCK,
                              "Could not save lock information.",
                              err);
    }

    return NULL;
}