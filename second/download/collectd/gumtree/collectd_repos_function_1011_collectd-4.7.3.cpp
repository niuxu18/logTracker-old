static int dispatch_values (const data_set_t *ds, value_list_t *vl,
	       	FILE *fh, char *buffer)
{
	char *dummy;
	char *ptr;
	char *saveptr;
	int i;

	char *time_str = buffer;
	char *value_str = strchr (time_str, ':');
	if (value_str == NULL)
	{
		print_to_socket (fh, "-1 No time found.\n");
		return (-1);
	}
	*value_str = '\0'; value_str++;

	vl->time = (time_t) atoi (time_str);

	i = 0;
	dummy = value_str;
	saveptr = NULL;
	while ((ptr = strtok_r (dummy, ":", &saveptr)) != NULL)
	{
		dummy = NULL;

		if (i >= vl->values_len)
		{
			i = vl->values_len + 1;
			break;
		}

		if ((strcmp (ptr, "U") == 0) && (ds->ds[i].type == DS_TYPE_GAUGE))
			vl->values[i].gauge = NAN;
		else if (0 != parse_value (ptr, &vl->values[i], ds->ds[i]))
		{
			print_to_socket (fh, "-1 Failed to parse value `%s'.\n", ptr);
			return (-1);
		}

		i++;
	} /* while (strtok_r) */

	if (i != vl->values_len)
	{
		char identifier[128];
		FORMAT_VL (identifier, sizeof (identifier), vl, ds);
		ERROR ("cmd putval: dispatch_values: "
				"Number of values incorrect: "
				"Got %i, expected %i. Identifier is `%s'.",
				i, vl->values_len, identifier);
		print_to_socket (fh, "-1 Number of values incorrect: "
				"Got %i, expected %i.\n",
			       	i, vl->values_len);
		return (-1);
	}

	plugin_dispatch_values (vl);
	return (0);
}