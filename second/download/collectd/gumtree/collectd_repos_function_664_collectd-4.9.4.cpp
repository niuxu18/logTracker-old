static PyObject *Notification_dispatch(Notification *self, PyObject *args, PyObject *kwds) {
	int ret;
	const data_set_t *ds;
	notification_t notification;
	double t = self->data.time;
	int severity = self->severity;
	const char *host = self->data.host;
	const char *plugin = self->data.plugin;
	const char *plugin_instance = self->data.plugin_instance;
	const char *type = self->data.type;
	const char *type_instance = self->data.type_instance;
	const char *message = self->message;
	
	static char *kwlist[] = {"type", "message", "plugin_instance", "type_instance",
			"plugin", "host", "time", "severity", NULL};
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ssssssdi", kwlist,
			&type, &message, &plugin_instance, &type_instance,
			&plugin, &host, &t, &severity))
		return NULL;

	if (type[0] == 0) {
		PyErr_SetString(PyExc_RuntimeError, "type not set");
		return NULL;
	}
	ds = plugin_get_ds(type);
	if (ds == NULL) {
		PyErr_Format(PyExc_TypeError, "Dataset %s not found", type);
		return NULL;
	}

	notification.time = t;
	notification.severity = severity;
	sstrncpy(notification.message, message, sizeof(notification.message));
	sstrncpy(notification.host, host, sizeof(notification.host));
	sstrncpy(notification.plugin, plugin, sizeof(notification.plugin));
	sstrncpy(notification.plugin_instance, plugin_instance, sizeof(notification.plugin_instance));
	sstrncpy(notification.type, type, sizeof(notification.type));
	sstrncpy(notification.type_instance, type_instance, sizeof(notification.type_instance));
	notification.meta = NULL;
	if (notification.time < 1)
		notification.time = time(0);
	if (notification.host[0] == 0)
		sstrncpy(notification.host, hostname_g, sizeof(notification.host));
	if (notification.plugin[0] == 0)
		sstrncpy(notification.plugin, "python", sizeof(notification.plugin));
	Py_BEGIN_ALLOW_THREADS;
	ret = plugin_dispatch_notification(&notification);
	Py_END_ALLOW_THREADS;
	if (ret != 0) {
		PyErr_SetString(PyExc_RuntimeError, "error dispatching notification, read the logs");
		return NULL;
	}
	Py_RETURN_NONE;
}