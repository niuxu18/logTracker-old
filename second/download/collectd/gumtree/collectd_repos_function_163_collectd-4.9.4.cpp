static int cdbi_config_add_database (oconfig_item_t *ci) /* {{{ */
{
  cdbi_database_t *db;
  int status;
  int i;

  if ((ci->values_num != 1)
      || (ci->values[0].type != OCONFIG_TYPE_STRING))
  {
    WARNING ("dbi plugin: The `Database' block "
        "needs exactly one string argument.");
    return (-1);
  }

  db = (cdbi_database_t *) malloc (sizeof (*db));
  if (db == NULL)
  {
    ERROR ("dbi plugin: malloc failed.");
    return (-1);
  }
  memset (db, 0, sizeof (*db));

  status = cdbi_config_set_string (&db->name, ci);
  if (status != 0)
  {
    sfree (db);
    return (status);
  }

  /* Fill the `cdbi_database_t' structure.. */
  for (i = 0; i < ci->children_num; i++)
  {
    oconfig_item_t *child = ci->children + i;

    if (strcasecmp ("Driver", child->key) == 0)
      status = cdbi_config_set_string (&db->driver, child);
    else if (strcasecmp ("DriverOption", child->key) == 0)
      status = cdbi_config_add_database_driver_option (db, child);
    else if (strcasecmp ("SelectDB", child->key) == 0)
      status = cdbi_config_set_string (&db->select_db, child);
    else if (strcasecmp ("Query", child->key) == 0)
      status = udb_query_pick_from_list (child, queries, queries_num,
          &db->queries, &db->queries_num);
    else
    {
      WARNING ("dbi plugin: Option `%s' not allowed here.", child->key);
      status = -1;
    }

    if (status != 0)
      break;
  }

  /* Check that all necessary options have been given. */
  while (status == 0)
  {
    if (db->driver == NULL)
    {
      WARNING ("dbi plugin: `Driver' not given for database `%s'", db->name);
      status = -1;
    }
    if (db->driver_options_num == 0)
    {
      WARNING ("dbi plugin: No `DriverOption' given for database `%s'. "
          "This will likely not work.", db->name);
    }

    break;
  } /* while (status == 0) */

  /* If all went well, add this database to the global list of databases. */
  if (status == 0)
  {
    cdbi_database_t **temp;

    temp = (cdbi_database_t **) realloc (databases,
        sizeof (*databases) * (databases_num + 1));
    if (temp == NULL)
    {
      ERROR ("dbi plugin: realloc failed");
      status = -1;
    }
    else
    {
      databases = temp;
      databases[databases_num] = db;
      databases_num++;
    }
  }

  if (status != 0)
  {
    cdbi_database_free (db);
    return (-1);
  }

  return (0);
}