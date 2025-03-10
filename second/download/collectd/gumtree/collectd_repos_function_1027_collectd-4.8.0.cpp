static int wh_write_command (const data_set_t *ds, const value_list_t *vl, /* {{{ */
                wh_callback_t *cb)
{
        char key[10*DATA_MAX_NAME_LEN];
        char values[512];
        char command[1024];
        size_t command_len;

        int status;

        if (0 != strcmp (ds->type, vl->type)) {
                ERROR ("write_http plugin: DS type does not match "
                                "value list type");
                return -1;
        }

        /* Copy the identifier to `key' and escape it. */
        status = FORMAT_VL (key, sizeof (key), vl);
        if (status != 0) {
                ERROR ("write_http plugin: error with format_name");
                return (status);
        }
        escape_string (key, sizeof (key));

        /* Convert the values to an ASCII representation and put that into
         * `values'. */
        status = wh_value_list_to_string (values, sizeof (values), ds, vl);
        if (status != 0) {
                ERROR ("write_http plugin: error with "
                                "wh_value_list_to_string");
                return (status);
        }

        command_len = (size_t) ssnprintf (command, sizeof (command),
                        "PUTVAL %s interval=%i %s\r\n",
                        key, vl->interval, values);
        if (command_len >= sizeof (command)) {
                ERROR ("write_http plugin: Command buffer too small: "
                                "Need %zu bytes.", command_len + 1);
                return (-1);
        }

        pthread_mutex_lock (&cb->send_lock);

        if (cb->curl == NULL)
        {
                status = wh_callback_init (cb);
                if (status != 0)
                {
                        ERROR ("write_http plugin: wh_callback_init failed.");
                        pthread_mutex_unlock (&cb->send_lock);
                        return (-1);
                }
        }

        if (command_len >= cb->send_buffer_free)
        {
                status = wh_flush_nolock (/* timeout = */ -1, cb);
                if (status != 0)
                {
                        pthread_mutex_unlock (&cb->send_lock);
                        return (status);
                }
        }
        assert (command_len < cb->send_buffer_free);

        /* `command_len + 1' because `command_len' does not include the
         * trailing null byte. Neither does `send_buffer_fill'. */
        memcpy (cb->send_buffer + cb->send_buffer_fill,
                        command, command_len + 1);
        cb->send_buffer_fill += command_len;
        cb->send_buffer_free -= command_len;

        DEBUG ("write_http plugin: <%s> buffer %zu/%zu (%g%%) \"%s\"",
                        cb->location,
                        cb->send_buffer_fill, sizeof (cb->send_buffer),
                        100.0 * ((double) cb->send_buffer_fill) / ((double) sizeof (cb->send_buffer)),
                        command);

        /* Check if we have enough space for this command. */
        pthread_mutex_unlock (&cb->send_lock);

        return (0);
}