int plugin_register_shutdown (char *name,
		int (*callback) (void))
{
	return (create_register_callback (&list_shutdown, name,
				(void *) callback, /* user_data = */ NULL));
}