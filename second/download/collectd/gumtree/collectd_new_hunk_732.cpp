
static int Notification_init(PyObject *s, PyObject *args, PyObject *kwds) {
  Notification *self = (Notification *)s;
  int severity = 0;
  double time = 0;
  char *message = NULL;
  PyObject *meta = NULL;
  char *type = NULL, *plugin_instance = NULL, *type_instance = NULL,
       *plugin = NULL, *host = NULL;
  static char *kwlist[] = {
      "type", "message", "plugin_instance", "type_instance", "plugin",
      "host", "time",    "severity",        "meta",          NULL};

  if (!PyArg_ParseTupleAndKeywords(
          args, kwds, "|etetetetetetdiO", kwlist, NULL, &type, NULL, &message,
          NULL, &plugin_instance, NULL, &type_instance, NULL, &plugin, NULL,
          &host, &time, &severity, &meta))
    return -1;

  if (type && plugin_get_ds(type) == NULL) {
    PyErr_Format(PyExc_TypeError, "Dataset %s not found", type);
    FreeAll();
    PyMem_Free(message);
