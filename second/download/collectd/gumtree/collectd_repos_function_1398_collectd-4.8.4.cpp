void plugin_shutdown_all (void)
{
	llentry_t *le;

	stop_read_threads ();

	destroy_all_callbacks (&list_init);

	pthread_mutex_lock (&read_lock);
	llist_destroy (read_list);
	read_list = NULL;
	pthread_mutex_unlock (&read_lock);

	destroy_read_heap ();

	plugin_flush (/* plugin = */ NULL, /* timeout = */ -1,
			/* identifier = */ NULL);

	le = NULL;
	if (list_shutdown != NULL)
		le = llist_head (list_shutdown);

	while (le != NULL)
	{
		callback_func_t *cf;
		plugin_shutdown_cb callback;

		cf = le->value;
		callback = cf->cf_callback;

		/* Advance the pointer before calling the callback allows
		 * shutdown functions to unregister themselves. If done the
		 * other way around the memory `le' points to will be freed
		 * after callback returns. */
		le = le->next;

		(*callback) ();
	}

	/* Write plugins which use the `user_data' pointer usually need the
	 * same data available to the flush callback. If this is the case, set
	 * the free_function to NULL when registering the flush callback and to
	 * the real free function when registering the write callback. This way
	 * the data isn't freed twice. */
	destroy_all_callbacks (&list_flush);
	destroy_all_callbacks (&list_write);

	destroy_all_callbacks (&list_notification);
	destroy_all_callbacks (&list_shutdown);
	destroy_all_callbacks (&list_log);
}