static int register_callback (llist_t **list, const char *name, void *callback)
{
	llentry_t *le;
	char *key;

	if ((*list == NULL)
			&& ((*list = llist_create ()) == NULL))
		return (-1);

	le = llist_search (*list, name);
	if (le == NULL)
	{
		key = strdup (name);
		if (key == NULL)
			return (-1);

		le = llentry_create (key, callback);
		if (le == NULL)
		{
			free (key);
			return (-1);
		}

		llist_append (*list, le);
	}
	else
	{
		le->value = callback;
	}

	return (0);
}