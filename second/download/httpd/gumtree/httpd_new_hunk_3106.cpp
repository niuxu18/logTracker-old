    int rows = 0;

    session_dbd_dir_conf *conf = ap_get_module_config(r->per_dir_config,
                                                      &session_dbd_module);
    ap_dbd_t *dbd = session_dbd_acquire_fn(r);
    if (dbd == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01861)
                      "failed to acquire database connection to remove "
                      "session with key '%s'", key);
        return APR_EGENERAL;
    }

    if (conf->deletelabel == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01862)
                      "no SessionDBDdeletelabel has been specified");
        return APR_EGENERAL;
    }

    statement = apr_hash_get(dbd->prepared, conf->deletelabel,
                             APR_HASH_KEY_STRING);
    if (statement == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01863)
                      "prepared statement could not be found for "
                      "SessionDBDdeletelabel with the label '%s'",
                      conf->deletelabel);
        return APR_EGENERAL;
    }
    rv = apr_dbd_pvbquery(dbd->driver, r->pool, dbd->handle, &rows, statement,
                          key, NULL);
    if (rv != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01864)
                      "query execution error removing session '%s' "
                      "from database", key);
        return rv;
    }

    return APR_SUCCESS;
