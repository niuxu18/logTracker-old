int handle_putval(FILE *fh, char *buffer) {
  char *command;
  char *identifier;
  char *hostname;
  char *plugin;
  char *plugin_instance;
  char *type;
  char *type_instance;
  int status;
  int values_submitted;

  char *identifier_copy;

  const data_set_t *ds;
  value_list_t vl = VALUE_LIST_INIT;
  vl.values = NULL;

  DEBUG("utils_cmd_putval: handle_putval (fh = %p, buffer = %s);", (void *)fh,
        buffer);

  command = NULL;
  status = parse_string(&buffer, &command);
  if (status != 0) {
    print_to_socket(fh, "-1 Cannot parse command.\n");
    return (-1);
  }
  assert(command != NULL);

  if (strcasecmp("PUTVAL", command) != 0) {
    print_to_socket(fh, "-1 Unexpected command: `%s'.\n", command);
    return (-1);
  }

  identifier = NULL;
  status = parse_string(&buffer, &identifier);
  if (status != 0) {
    print_to_socket(fh, "-1 Cannot parse identifier.\n");
    return (-1);
  }
  assert(identifier != NULL);

  /* parse_identifier() modifies its first argument,
   * returning pointers into it */
  identifier_copy = sstrdup(identifier);

  status = parse_identifier(identifier_copy, &hostname, &plugin,
                            &plugin_instance, &type, &type_instance);
  if (status != 0) {
    DEBUG("handle_putval: Cannot parse identifier `%s'.", identifier);
    print_to_socket(fh, "-1 Cannot parse identifier `%s'.\n", identifier);
    sfree(identifier_copy);
    return (-1);
  }

  if ((strlen(hostname) >= sizeof(vl.host)) ||
      (strlen(plugin) >= sizeof(vl.plugin)) ||
      ((plugin_instance != NULL) &&
       (strlen(plugin_instance) >= sizeof(vl.plugin_instance))) ||
      ((type_instance != NULL) &&
       (strlen(type_instance) >= sizeof(vl.type_instance)))) {
    print_to_socket(fh, "-1 Identifier too long.\n");
    sfree(identifier_copy);
    return (-1);
  }

  sstrncpy(vl.host, hostname, sizeof(vl.host));
  sstrncpy(vl.plugin, plugin, sizeof(vl.plugin));
  sstrncpy(vl.type, type, sizeof(vl.type));
  if (plugin_instance != NULL)
    sstrncpy(vl.plugin_instance, plugin_instance, sizeof(vl.plugin_instance));
  if (type_instance != NULL)
    sstrncpy(vl.type_instance, type_instance, sizeof(vl.type_instance));

  ds = plugin_get_ds(type);
  if (ds == NULL) {
    print_to_socket(fh, "-1 Type `%s' isn't defined.\n", type);
    sfree(identifier_copy);
    return (-1);
  }

  /* Free identifier_copy */
  hostname = NULL;
  plugin = NULL;
  plugin_instance = NULL;
  type = NULL;
  type_instance = NULL;
  sfree(identifier_copy);

  vl.values_len = ds->ds_num;
  vl.values = malloc(vl.values_len * sizeof(*vl.values));
  if (vl.values == NULL) {
    print_to_socket(fh, "-1 malloc failed.\n");
    return (-1);
  }

  /* All the remaining fields are part of the optionlist. */
  values_submitted = 0;
  while (*buffer != 0) {
    char *string = NULL;
    char *value = NULL;

    status = parse_option(&buffer, &string, &value);
    if (status < 0) {
      /* parse_option failed, buffer has been modified.
       * => we need to abort */
      print_to_socket(fh, "-1 Misformatted option.\n");
      sfree(vl.values);
      return (-1);
    } else if (status == 0) {
      assert(string != NULL);
      assert(value != NULL);
      set_option(&vl, string, value);
      continue;
    }
    /* else: parse_option but buffer has not been modified. This is
     * the default if no `=' is found.. */

    status = parse_string(&buffer, &string);
    if (status != 0) {
      print_to_socket(fh, "-1 Misformatted value.\n");
      sfree(vl.values);
      return (-1);
    }
    assert(string != NULL);

    status = parse_values(string, &vl, ds);
    if (status != 0) {
      print_to_socket(fh, "-1 Parsing the values string failed.\n");
      sfree(vl.values);
      return (-1);
    }

    plugin_dispatch_values(&vl);
    values_submitted++;
  } /* while (*buffer != 0) */
  /* Done parsing the options. */

  if (fh != stdout)
    print_to_socket(fh, "0 Success: %i %s been dispatched.\n", values_submitted,
                    (values_submitted == 1) ? "value has" : "values have");

  sfree(vl.values);
  return (0);
}