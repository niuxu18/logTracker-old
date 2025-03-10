void plugin_init_all (void)
{
	int (*callback) (void);
	llentry_t *le;
	int status;

	/* Init the value cache */
	uc_init ();

	if ((list_init == NULL) && (list_read == NULL))
		return;

	/* Calling all init callbacks before checking if read callbacks
	 * are available allows the init callbacks to register the read
	 * callback. */
	le = llist_head (list_init);
	while (le != NULL)
	{
		callback = (int (*) (void)) le->value;
		status = (*callback) ();

		if (status != 0)
		{
			ERROR ("Initialization of plugin `%s' "
					"failed with status %i. "
					"Plugin will be unloaded.",
					le->key, status);
			/* Plugins that register read callbacks from the init
			 * callback should take care of appropriate error
			 * handling themselves. */
			/* FIXME: Unload _all_ functions */
			plugin_unregister_read (le->key);
		}

		le = le->next;
	}

	/* Start read-threads */
	if (list_read != NULL)
	{
		const char *rt;
		int num;
		rt = global_option_get ("ReadThreads");
		num = atoi (rt);
		start_threads ((num > 0) ? num : 5);
	}
}