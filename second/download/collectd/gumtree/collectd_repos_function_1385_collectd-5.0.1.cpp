int handle_flush (FILE *fh, char *buffer)
{
	int success = 0;
	int error   = 0;

	double timeout = 0.0;
	char **plugins = NULL;
	int plugins_num = 0;
	char **identifiers = NULL;
	int identifiers_num = 0;

	int i;

	if ((fh == NULL) || (buffer == NULL))
		return (-1);

	DEBUG ("utils_cmd_flush: handle_flush (fh = %p, buffer = %s);",
			(void *) fh, buffer);

	if (strncasecmp ("FLUSH", buffer, strlen ("FLUSH")) != 0)
	{
		print_to_socket (fh, "-1 Cannot parse command.\n");
		return (-1);
	}
	buffer += strlen ("FLUSH");

	while (*buffer != 0)
	{
		char *opt_key;
		char *opt_value;
		int status;

		opt_key = NULL;
		opt_value = NULL;
		status = parse_option (&buffer, &opt_key, &opt_value);
		if (status != 0)
		{
			print_to_socket (fh, "-1 Parsing options failed.\n");
			sfree (plugins);
			sfree (identifiers);
			return (-1);
		}

		if (strcasecmp ("plugin", opt_key) == 0)
		{
			add_to_array (&plugins, &plugins_num, opt_value);
		}
		else if (strcasecmp ("identifier", opt_key) == 0)
		{
			add_to_array (&identifiers, &identifiers_num, opt_value);
		}
		else if (strcasecmp ("timeout", opt_key) == 0)
		{
			char *endptr;
			
			errno = 0;
			endptr = NULL;
			timeout = strtod (opt_value, &endptr);

			if ((endptr == opt_value) || (errno != 0) || (!isfinite (timeout)))
			{
				print_to_socket (fh, "-1 Invalid value for option `timeout': "
						"%s\n", opt_value);
				sfree (plugins);
				sfree (identifiers);
				return (-1);
			}
			else if (timeout < 0.0)
			{
				timeout = 0.0;
			}
		}
		else
		{
			print_to_socket (fh, "-1 Cannot parse option %s\n", opt_key);
			sfree (plugins);
			sfree (identifiers);
			return (-1);
		}
	} /* while (*buffer != 0) */

	/* Add NULL entries for `any plugin' and/or `any value' if nothing was
	 * specified. */
	if (plugins_num == 0)
		add_to_array (&plugins, &plugins_num, NULL);

	if (identifiers_num == 0)
		add_to_array (&identifiers, &identifiers_num, NULL);

	for (i = 0; i < plugins_num; i++)
	{
		char *plugin;
		int j;

		plugin = plugins[i];

		for (j = 0; j < identifiers_num; j++)
		{
			char *identifier;
			int status;

			identifier = identifiers[j];
			status = plugin_flush (plugin,
					DOUBLE_TO_CDTIME_T (timeout),
					identifier);
			if (status == 0)
				success++;
			else
				error++;
		}
	}

	if ((success + error) > 0)
	{
		print_to_socket (fh, "0 Done: %i successful, %i errors\n",
				success, error);
	}
	else
	{
		plugin_flush (NULL, timeout, NULL);
		print_to_socket (fh, "0 Done\n");
	}

	sfree (plugins);
	sfree (identifiers);
	return (0);
}