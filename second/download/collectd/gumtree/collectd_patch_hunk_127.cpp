 		"Notifications can be dispatched at any time and can be received with register_notification.";
 
 static int Notification_init(PyObject *s, PyObject *args, PyObject *kwds) {
 	Notification *self = (Notification *) s;
 	int severity = 0;
 	double time = 0;
-	char *message = NULL;
-	char *type = NULL, *plugin_instance = NULL, *type_instance = NULL, *plugin = NULL, *host = NULL;
+	const char *message = "";
+	const char *type = "", *plugin_instance = "", *type_instance = "", *plugin = "", *host = "";
 	static char *kwlist[] = {"type", "message", "plugin_instance", "type_instance",
 			"plugin", "host", "time", "severity", NULL};
 	
 	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|etetetetetetdi", kwlist,
 			NULL, &type, NULL, &message, NULL, &plugin_instance, NULL, &type_instance,
 			NULL, &plugin, NULL, &host, &time, &severity))
 		return -1;
 	
-	if (type && plugin_get_ds(type) == NULL) {
+	if (type[0] != 0 && plugin_get_ds(type) == NULL) {
 		PyErr_Format(PyExc_TypeError, "Dataset %s not found", type);
-		FreeAll();
-		PyMem_Free(message);
 		return -1;
 	}
 
-	sstrncpy(self->data.host, host ? host : "", sizeof(self->data.host));
-	sstrncpy(self->data.plugin, plugin ? plugin : "", sizeof(self->data.plugin));
-	sstrncpy(self->data.plugin_instance, plugin_instance ? plugin_instance : "", sizeof(self->data.plugin_instance));
-	sstrncpy(self->data.type, type ? type : "", sizeof(self->data.type));
-	sstrncpy(self->data.type_instance, type_instance ? type_instance : "", sizeof(self->data.type_instance));
-	sstrncpy(self->message, message ? message : "", sizeof(self->message));
+	sstrncpy(self->data.host, host, sizeof(self->data.host));
+	sstrncpy(self->data.plugin, plugin, sizeof(self->data.plugin));
+	sstrncpy(self->data.plugin_instance, plugin_instance, sizeof(self->data.plugin_instance));
+	sstrncpy(self->data.type, type, sizeof(self->data.type));
+	sstrncpy(self->data.type_instance, type_instance, sizeof(self->data.type_instance));
 	self->data.time = time;
-	self->severity = severity;
 
-	FreeAll();
-	PyMem_Free(message);
+	sstrncpy(self->message, message, sizeof(self->message));
+	self->severity = severity;
 	return 0;
 }
 
 static PyObject *Notification_dispatch(Notification *self, PyObject *args, PyObject *kwds) {
 	int ret;
 	const data_set_t *ds;
 	notification_t notification;
 	double t = self->data.time;
 	int severity = self->severity;
-	char *host = NULL, *plugin = NULL, *plugin_instance = NULL, *type = NULL, *type_instance = NULL;
-	char *message = NULL;
+	const char *host = self->data.host;
+	const char *plugin = self->data.plugin;
+	const char *plugin_instance = self->data.plugin_instance;
+	const char *type = self->data.type;
+	const char *type_instance = self->data.type_instance;
+	const char *message = self->message;
 	
 	static char *kwlist[] = {"type", "message", "plugin_instance", "type_instance",
 			"plugin", "host", "time", "severity", NULL};
 	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|etetetetetetdi", kwlist,
 			NULL, &type, NULL, &message, NULL, &plugin_instance, NULL, &type_instance,
 			NULL, &plugin, NULL, &host, &t, &severity))
 		return NULL;
 
-	notification.time = t;
-	notification.severity = severity;
-	sstrncpy(notification.message, message ? message : self->message, sizeof(notification.message));
-	sstrncpy(notification.host, host ? host : self->data.host, sizeof(notification.host));
-	sstrncpy(notification.plugin, plugin ? plugin : self->data.plugin, sizeof(notification.plugin));
-	sstrncpy(notification.plugin_instance, plugin_instance ? plugin_instance : self->data.plugin_instance, sizeof(notification.plugin_instance));
-	sstrncpy(notification.type, type ? type : self->data.type, sizeof(notification.type));
-	sstrncpy(notification.type_instance, type_instance ? type_instance : self->data.type_instance, sizeof(notification.type_instance));
-	notification.meta = NULL;
-	FreeAll();
-	PyMem_Free(message);
-
-	if (notification.type[0] == 0) {
+	if (type[0] == 0) {
 		PyErr_SetString(PyExc_RuntimeError, "type not set");
 		return NULL;
 	}
-	ds = plugin_get_ds(notification.type);
+	ds = plugin_get_ds(type);
 	if (ds == NULL) {
-		PyErr_Format(PyExc_TypeError, "Dataset %s not found", notification.type);
+		PyErr_Format(PyExc_TypeError, "Dataset %s not found", type);
 		return NULL;
 	}
 
-	if (notification.time < 1)
-		notification.time = time(0);
+	notification.time = DOUBLE_TO_CDTIME_T(t);
+	notification.severity = severity;
+	sstrncpy(notification.message, message, sizeof(notification.message));
+	sstrncpy(notification.host, host, sizeof(notification.host));
+	sstrncpy(notification.plugin, plugin, sizeof(notification.plugin));
+	sstrncpy(notification.plugin_instance, plugin_instance, sizeof(notification.plugin_instance));
+	sstrncpy(notification.type, type, sizeof(notification.type));
+	sstrncpy(notification.type_instance, type_instance, sizeof(notification.type_instance));
+	notification.meta = NULL;
+	if (notification.time == 0)
+		notification.time = cdtime();
 	if (notification.host[0] == 0)
 		sstrncpy(notification.host, hostname_g, sizeof(notification.host));
 	if (notification.plugin[0] == 0)
 		sstrncpy(notification.plugin, "python", sizeof(notification.plugin));
 	Py_BEGIN_ALLOW_THREADS;
 	ret = plugin_dispatch_notification(&notification);
