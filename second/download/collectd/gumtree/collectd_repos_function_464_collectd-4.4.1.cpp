static void load_submit (const char *plugin_instance,
		gauge_t snum, gauge_t mnum, gauge_t lnum)
{
	value_t values[3];
	value_list_t vl = VALUE_LIST_INIT;

	values[0].gauge = snum;
	values[1].gauge = mnum;
	values[2].gauge = lnum;

	vl.values = values;
	vl.values_len = STATIC_ARRAY_SIZE (values);
	vl.time = time (NULL);
	strcpy (vl.host, hostname_g);
	strcpy (vl.plugin, "vserver");
	strncpy (vl.plugin_instance, plugin_instance, sizeof (vl.plugin_instance));

	plugin_dispatch_values ("load", &vl);
}