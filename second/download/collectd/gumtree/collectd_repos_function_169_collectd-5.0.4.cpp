static int cdbi_connect_database (cdbi_database_t *db) /* {{{ */
{
  dbi_driver driver;
  dbi_conn connection;
  size_t i;
  int status;

  if (db->connection != NULL)
  {
    status = dbi_conn_ping (db->connection);
    if (status != 0) /* connection is alive */
      return (0);

    dbi_conn_close (db->connection);
    db->connection = NULL;
  }

  driver = dbi_driver_open (db->driver);
  if (driver == NULL)
  {
    ERROR ("dbi plugin: cdbi_connect_database: dbi_driver_open (%s) failed.",
        db->driver);
    INFO ("dbi plugin: Maybe the driver isn't installed? "
        "Known drivers are:");
    for (driver = dbi_driver_list (NULL);
        driver != NULL;
        driver = dbi_driver_list (driver))
    {
      INFO ("dbi plugin: * %s", dbi_driver_get_name (driver));
    }
    return (-1);
  }

  connection = dbi_conn_open (driver);
  if (connection == NULL)
  {
    ERROR ("dbi plugin: cdbi_connect_database: dbi_conn_open (%s) failed.",
        db->driver);
    return (-1);
  }

  /* Set all the driver options. Because this is a very very very generic
   * interface, the error handling is kind of long. If an invalid option is
   * encountered, it will get a list of options understood by the driver and
   * report that as `INFO'. This way, users hopefully don't have too much
   * trouble finding out how to configure the plugin correctly.. */
  for (i = 0; i < db->driver_options_num; i++)
  {
    DEBUG ("dbi plugin: cdbi_connect_database (%s): "
        "key = %s; value = %s;",
        db->name,
        db->driver_options[i].key,
        db->driver_options[i].value);

    status = dbi_conn_set_option (connection,
        db->driver_options[i].key, db->driver_options[i].value);
    if (status != 0)
    {
      char errbuf[1024];
      const char *opt;

      ERROR ("dbi plugin: cdbi_connect_database (%s): "
          "dbi_conn_set_option (%s, %s) failed: %s.",
          db->name,
          db->driver_options[i].key, db->driver_options[i].value,
          cdbi_strerror (connection, errbuf, sizeof (errbuf)));

      INFO ("dbi plugin: This is a list of all options understood "
          "by the `%s' driver:", db->driver);
      for (opt = dbi_conn_get_option_list (connection, NULL);
          opt != NULL;
          opt = dbi_conn_get_option_list (connection, opt))
      {
        INFO ("dbi plugin: * %s", opt);
      }

      dbi_conn_close (connection);
      return (-1);
    }
  } /* for (i = 0; i < db->driver_options_num; i++) */

  status = dbi_conn_connect (connection);
  if (status != 0)
  {
    char errbuf[1024];
    ERROR ("dbi plugin: cdbi_connect_database (%s): "
        "dbi_conn_connect failed: %s",
        db->name, cdbi_strerror (connection, errbuf, sizeof (errbuf)));
    dbi_conn_close (connection);
    return (-1);
  }

  if (db->select_db != NULL)
  {
    status = dbi_conn_select_db (connection, db->select_db);
    if (status != 0)
    {
      char errbuf[1024];
      WARNING ("dbi plugin: cdbi_connect_database (%s): "
          "dbi_conn_select_db (%s) failed: %s. Check the `SelectDB' option.",
          db->name, db->select_db,
          cdbi_strerror (connection, errbuf, sizeof (errbuf)));
      dbi_conn_close (connection);
      return (-1);
    }
  }

  db->connection = connection;
  return (0);
}