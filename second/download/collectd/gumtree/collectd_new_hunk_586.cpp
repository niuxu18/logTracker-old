
	Py_DECREF(arg);
	if (tmp == NULL) {
		PyErr_Format(PyExc_RuntimeError, "Unable to unregister %s callback '%s'.", desc, name);
		return NULL;
	}
	/* Yes, this is actually safe. To call this function the caller has to
	 * hold the GIL. Well, safe as long as there is only one GIL anyway ... */
	if (prev == NULL)
		*list_head = tmp->next;
	else
		prev->next = tmp->next;
	cpy_destroy_user_data(tmp);
	Py_RETURN_NONE;
}

static void cpy_unregister_list(cpy_callback_t **list_head) {
	cpy_callback_t *cur, *next;
	for (cur = *list_head; cur; cur = next) {
		next = cur->next;
		cpy_destroy_user_data(cur);
	}
	*list_head = NULL;
}

typedef int cpy_unregister_function_t(const char *name);

static PyObject *cpy_unregister_generic_userdata(cpy_unregister_function_t *unreg, PyObject *arg, const char *desc) {
	char buf[512];
	const char *name;

