static int dbd_sqlite3_pselect(apr_pool_t *pool, apr_dbd_t *sql,
                               apr_dbd_results_t **results,
                               apr_dbd_prepared_t *statement, int seek,
                               int nargs, const char **values)
{
    return APR_ENOTIMPL;
}