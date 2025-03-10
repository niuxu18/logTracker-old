static int hddtemp_config (const char *key, const char *value)
{
	if (strcasecmp (key, "Host") == 0)
	{
		if (hddtemp_host != NULL)
			free (hddtemp_host);
		hddtemp_host = strdup (value);
	}
	else if (strcasecmp (key, "Port") == 0)
	{
		int port = (int) (atof (value));
		if ((port > 0) && (port <= 65535))
			snprintf (hddtemp_port, sizeof (hddtemp_port),
					"%i", port);
		else
			strncpy (hddtemp_port, value, sizeof (hddtemp_port));
		hddtemp_port[sizeof (hddtemp_port) - 1] = '\0';
	}
	else if (strcasecmp (key, "TranslateDevicename") == 0)
	{
		if ((strcasecmp ("true", value) == 0)
				|| (strcasecmp ("yes", value) == 0)
				|| (strcasecmp ("on", value) == 0))
			translate_devicename = 1;
		else
			translate_devicename = 0;
	}
	else
	{
		return (-1);
	}

	return (0);
}