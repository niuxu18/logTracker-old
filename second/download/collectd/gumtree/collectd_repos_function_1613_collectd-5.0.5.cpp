static PyObject *cpy_register_read(PyObject *self, PyObject *args, PyObject *kwds) {
	char buf[512];
	cpy_callback_t *c = NULL;
	user_data_t *user_data = NULL;
	double interval = 0;
	char *name = NULL;
	PyObject *callback = NULL, *data = NULL;
	struct timespec ts;
	static char *kwlist[] = {"callback", "interval", "data", "name", NULL};
	
	if (PyArg_ParseTupleAndKeywords(args, kwds, "O|dOet", kwlist, &callback, &interval, &data, NULL, &name) == 0) return NULL;
	if (PyCallable_Check(callback) == 0) {
		PyMem_Free(name);
		PyErr_SetString(PyExc_TypeError, "callback needs a be a callable object.");
		return NULL;
	}
	cpy_build_name(buf, sizeof(buf), callback, name);
	PyMem_Free(name);
	
	Py_INCREF(callback);
	Py_XINCREF(data);
	c = malloc(sizeof(*c));
	c->name = strdup(buf);
	c->callback = callback;
	c->data = data;
	c->next = NULL;
	user_data = malloc(sizeof(*user_data));
	user_data->free_func = cpy_destroy_user_data;
	user_data->data = c;
	ts.tv_sec = interval;
	ts.tv_nsec = (interval - ts.tv_sec) * 1000000000;
	plugin_register_complex_read(/* group = */ NULL, buf,
			cpy_read_callback, &ts, user_data);
	return cpy_string_to_unicode_or_bytes(buf);
}