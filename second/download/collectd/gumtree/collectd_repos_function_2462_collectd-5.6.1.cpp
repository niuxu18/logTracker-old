int tail_match_add_match_simple (cu_tail_match_t *obj,
    const char *regex, const char *excluderegex, int ds_type,
    const char *plugin, const char *plugin_instance,
    const char *type, const char *type_instance, const cdtime_t interval)
{
  cu_match_t *match;
  cu_tail_match_simple_t *user_data;
  int status;

  match = match_create_simple (regex, excluderegex, ds_type);
  if (match == NULL)
    return (-1);

  user_data = calloc (1, sizeof (*user_data));
  if (user_data == NULL)
  {
    match_destroy (match);
    return (-1);
  }

  sstrncpy (user_data->plugin, plugin, sizeof (user_data->plugin));
  if (plugin_instance != NULL)
    sstrncpy (user_data->plugin_instance, plugin_instance,
	sizeof (user_data->plugin_instance));

  sstrncpy (user_data->type, type, sizeof (user_data->type));
  if (type_instance != NULL)
    sstrncpy (user_data->type_instance, type_instance,
	sizeof (user_data->type_instance));

  user_data->interval = interval;

  status = tail_match_add_match (obj, match, simple_submit_match,
      user_data, free);

  if (status != 0)
  {
    match_destroy (match);
    sfree (user_data);
  }

  return (status);
}