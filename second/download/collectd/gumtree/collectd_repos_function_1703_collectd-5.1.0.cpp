static int wg_write_messages (const data_set_t *ds, const value_list_t *vl,
        struct wg_callback *cb)
{
    char key[10*DATA_MAX_NAME_LEN];
    char values[512];

    int status, i;

    if (0 != strcmp (ds->type, vl->type))
    {
        ERROR ("write_graphite plugin: DS type does not match "
                "value list type");
        return -1;
    }

    for (i = 0; i < ds->ds_num; i++)
    {
        const char *ds_name = NULL;

        if (cb->always_append_ds || (ds->ds_num > 1))
            ds_name = ds->ds[i].name;

        /* Copy the identifier to `key' and escape it. */
        status = wg_format_name (key, sizeof (key), vl, cb, ds_name);
        if (status != 0)
        {
            ERROR ("write_graphite plugin: error with format_name");
            return (status);
        }

        escape_string (key, sizeof (key));
        /* Convert the values to an ASCII representation and put that into
         * `values'. */
        status = wg_format_values (values, sizeof (values), i, ds, vl,
                    cb->store_rates);
        if (status != 0)
        {
            ERROR ("write_graphite plugin: error with "
                    "wg_format_values");
            return (status);
        }

        /* Send the message to graphite */
        status = wg_send_message (key, values, vl->time, cb);
        if (status != 0)
        {
            ERROR ("write_graphite plugin: error with "
                    "wg_send_message");
            return (status);
        }
    }

    return (0);
}