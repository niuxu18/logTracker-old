static PyObject *cpy_register_generic(cpy_callback_t **list_head, PyObject *args, PyObject *kwds) {
	char buf[512];
	cpy_callback_t *c;
	char *name = NULL;
	PyObject *callback = NULL, *data = NULL, *mod = NULL;
	static char *kwlist[] = {"callback", "data", "name", NULL};
	
	if (PyArg_ParseTupleAndKeywords(args, kwds, "O|Oet", kwlist, &callback, &data, NULL, &name) == 0) return NULL;
	if (PyCallable_Check(callback) == 0) {
		PyMem_Free(name);
		PyErr_SetString(PyExc_TypeError, "callback needs a be a callable object.");
		return NULL;
	}
	cpy_build_name(buf, sizeof(buf), callback, name);

	Py_INCREF(callback);
	Py_XINCREF(data);
	c = malloc(sizeof(*c));
	c->name = strdup(buf);
	c->callback = callback;
	c->data = data;
	c->next = *list_head;
	*list_head = c;
	Py_XDECREF(mod);
	PyMem_Free(name);
	return cpy_string_to_unicode_or_bytes(buf);
}