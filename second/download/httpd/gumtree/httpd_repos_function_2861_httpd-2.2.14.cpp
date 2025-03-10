static int dbd_oracle_pvbselect(apr_pool_t * pool, apr_dbd_t * sql,
                                apr_dbd_results_t ** results,
                                apr_dbd_prepared_t * statement, int seek,
                                va_list args)
{
    const void **values;
    int i;

    if (sql->trans && sql->trans->status == TRANS_ERROR) {
        return -1;
    }

    values = apr_palloc(pool, sizeof(*values) * statement->nvals);

    for (i = 0; i < statement->nvals; i++) {
        values[i] = va_arg(args, const void*);
    }

    return dbd_oracle_pbselect(pool, sql, results, statement, seek, values);
}