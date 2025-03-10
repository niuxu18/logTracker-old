static oconfig_item_t *cf_read_dir (const char *dir, int depth)
{
	oconfig_item_t *root = NULL;
	DIR *dh;
	struct dirent *de;
	char **filenames = NULL;
	int filenames_num = 0;
	int status;
	int i;

	assert (depth < CF_MAX_DEPTH);

	dh = opendir (dir);
	if (dh == NULL)
	{
		char errbuf[1024];
		ERROR ("configfile: opendir failed: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (NULL);
	}

	root = (oconfig_item_t *) malloc (sizeof (oconfig_item_t));
	if (root == NULL)
	{
		ERROR ("configfile: malloc failed.");
		return (NULL);
	}
	memset (root, '\0', sizeof (oconfig_item_t));

	while ((de = readdir (dh)) != NULL)
	{
		char   name[1024];
		char **tmp;

		if ((de->d_name[0] == '.') || (de->d_name[0] == '\0'))
			continue;

		status = snprintf (name, sizeof (name), "%s/%s",
				dir, de->d_name);
		if (status >= sizeof (name))
		{
			ERROR ("configfile: Not including `%s/%s' because its"
					" name is too long.",
					dir, de->d_name);
			for (i = 0; i < filenames_num; ++i)
				free (filenames[i]);
			free (filenames);
			free (root);
			return (NULL);
		}

		++filenames_num;
		tmp = (char **) realloc (filenames,
				filenames_num * sizeof (*filenames));
		if (tmp == NULL) {
			ERROR ("configfile: realloc failed.");
			for (i = 0; i < filenames_num - 1; ++i)
				free (filenames[i]);
			free (filenames);
			free (root);
			return (NULL);
		}
		filenames = tmp;

		filenames[filenames_num - 1] = sstrdup (name);
	}

	qsort ((void *) filenames, filenames_num, sizeof (*filenames),
			cf_compare_string);

	for (i = 0; i < filenames_num; ++i)
	{
		oconfig_item_t *temp;
		char *name = filenames[i];

		temp = cf_read_generic (name, depth);
		if (temp == NULL) {
			int j;
			for (j = i; j < filenames_num; ++j)
				free (filenames[j]);
			free (filenames);
			oconfig_free (root);
			return (NULL);
		}

		cf_ci_append_children (root, temp);
		sfree (temp->children);
		sfree (temp);

		free (name);
	}

	free(filenames);
	return (root);
}