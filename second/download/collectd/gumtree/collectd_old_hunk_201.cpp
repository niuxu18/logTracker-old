		}
	} while (0);

	return (0);
} /* int us_open_socket */

static int us_handle_getval (FILE *fh, char **fields, int fields_num)
{
	char *hostname;
	char *plugin;
	char *plugin_instance;
	char *type;
	char *type_instance;
	char  name[4*DATA_MAX_NAME_LEN];
	value_cache_t *vc;
	int   status;
	int   i;

	if (fields_num != 2)
	{
		DEBUG ("unixsock plugin: Wrong number of fields: %i", fields_num);
		fprintf (fh, "-1 Wrong number of fields: Got %i, expected 2.\n",
				fields_num);
		fflush (fh);
		return (-1);
	}
	DEBUG ("unixsock plugin: Got query for `%s'", fields[1]);

	status = parse_identifier (fields[1], &hostname,
			&plugin, &plugin_instance,
			&type, &type_instance);
	if (status != 0)
	{
		DEBUG ("unixsock plugin: Cannot parse `%s'", fields[1]);
		fprintf (fh, "-1 Cannot parse identifier.\n");
		fflush (fh);
		return (-1);
	}

	status = format_name (name, sizeof (name),
			hostname, plugin, plugin_instance, type, type_instance);
	if (status != 0)
	{
		fprintf (fh, "-1 format_name failed.\n");
		return (-1);
	}

	pthread_mutex_lock (&cache_lock);

	DEBUG ("vc = cache_search (%s)", name);
	vc = cache_search (name);

	if (vc == NULL)
	{
		DEBUG ("Did not find cache entry.");
		fprintf (fh, "-1 No such value");
	}
	else
	{
		DEBUG ("Found cache entry.");
		fprintf (fh, "%i", vc->values_num);
		for (i = 0; i < vc->values_num; i++)
		{
			fprintf (fh, " %s=", vc->ds->ds[i].name);
			if (isnan (vc->gauge[i]))
				fprintf (fh, "NaN");
			else
				fprintf (fh, "%12e", vc->gauge[i]);
		}
	}

	/* Free the mutex as soon as possible and definitely before flushing */
	pthread_mutex_unlock (&cache_lock);

	fprintf (fh, "\n");
	fflush (fh);

	return (0);
} /* int us_handle_getval */

static int us_handle_listval (FILE *fh, char **fields, int fields_num)
{
	char buffer[1024];
	char **value_list = NULL;
	int value_list_len = 0;
	value_cache_t *entry;
	int i;

	if (fields_num != 1)
	{
		DEBUG ("unixsock plugin: us_handle_listval: "
				"Wrong number of fields: %i", fields_num);
		fprintf (fh, "-1 Wrong number of fields: Got %i, expected 1.\n",
				fields_num);
		fflush (fh);
		return (-1);
	}

	pthread_mutex_lock (&cache_lock);

	for (entry = cache_head; entry != NULL; entry = entry->next)
	{
		char **tmp;

		snprintf (buffer, sizeof (buffer), "%u %s\n",
				(unsigned int) entry->time, entry->name);
		buffer[sizeof (buffer) - 1] = '\0';
		
		tmp = realloc (value_list, sizeof (char *) * (value_list_len + 1));
		if (tmp == NULL)
			continue;
		value_list = tmp;

		value_list[value_list_len] = strdup (buffer);

		if (value_list[value_list_len] != NULL)
			value_list_len++;
	} /* for (entry) */

	pthread_mutex_unlock (&cache_lock);

	DEBUG ("unixsock plugin: us_handle_listval: value_list_len = %i", value_list_len);
	fprintf (fh, "%i Values found\n", value_list_len);
	for (i = 0; i < value_list_len; i++)
		fputs (value_list[i], fh);
	fflush (fh);

	return (0);
} /* int us_handle_listval */

static void *us_handle_client (void *arg)
{
	int fd;
	FILE *fhin, *fhout;
	char buffer[1024];
	char *fields[128];
