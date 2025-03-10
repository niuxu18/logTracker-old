static int resolve_symlink(char *d, apr_finfo_t *lfi, int opts, apr_pool_t *p)
{
    apr_finfo_t fi;
    int res;
    const char *savename;

    if (!(opts & (OPT_SYM_OWNER | OPT_SYM_LINKS))) {
        return HTTP_FORBIDDEN;
    }

    /* Save the name from the valid bits. */
    savename = (lfi->valid & APR_FINFO_NAME) ? lfi->name : NULL;

    if (opts & OPT_SYM_LINKS) {
        if ((res = apr_stat(&fi, d, lfi->valid & ~(APR_FINFO_NAME 
                                                 | APR_FINFO_LINK), p)) 
                 != APR_SUCCESS) {
            return HTTP_FORBIDDEN;
        }

        /* Give back the target */
        memcpy(lfi, &fi, sizeof(fi));
        if (savename) {
            lfi->name = savename;
            lfi->valid |= APR_FINFO_NAME;
        }

        return OK;
    }

    /* OPT_SYM_OWNER only works if we can get the owner of
     * both the file and symlink.  First fill in a missing
     * owner of the symlink, then get the info of the target.
     */
    if (!(lfi->valid & APR_FINFO_OWNER)) {
        if ((res = apr_lstat(&fi, d, lfi->valid | APR_FINFO_OWNER, p))
            != APR_SUCCESS) {
            return HTTP_FORBIDDEN;
        }
    }

    if ((res = apr_stat(&fi, d, lfi->valid & ~(APR_FINFO_NAME), p))
        != APR_SUCCESS) {
        return HTTP_FORBIDDEN;
    }

    if (apr_compare_users(fi.user, lfi->user) != APR_SUCCESS) {
        return HTTP_FORBIDDEN;
    }

    /* Give back the target */
    memcpy(lfi, &fi, sizeof(fi));
    if (savename) {
        lfi->name = savename;
        lfi->valid |= APR_FINFO_NAME;
    }

    return OK;
}