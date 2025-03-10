static void submit_gauge2 (const char *type, const char *type_inst,
    gauge_t value0, gauge_t value1, memcached_t *st)
{
  value_t values[2];
  value_list_t vl = VALUE_LIST_INIT;
  memcached_init_vl (&vl, st);

  values[0].gauge = value0;
  values[1].gauge = value1;

  vl.values = values;
  vl.values_len = 2;
  sstrncpy (vl.type, type, sizeof (vl.type));
  if (type_inst != NULL)
    sstrncpy (vl.type_instance, type_inst, sizeof (vl.type_instance));

  plugin_dispatch_values (&vl);
}