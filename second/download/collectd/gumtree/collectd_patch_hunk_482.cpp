 
 static meta_data_t *cpy_build_meta(PyObject *meta) {
 	int i, s;
 	meta_data_t *m = NULL;
 	PyObject *l;
 	
-	if ((meta == NULL) || (meta == Py_None))
+	if (!meta)
 		return NULL;
 
 	l = PyDict_Items(meta); /* New reference. */
 	if (!l) {
 		cpy_log_exception("building meta data");
 		return NULL;
 	}
-	s = PyList_Size(l);
-	if (s <= 0) {
-		Py_XDECREF(l);
-		return NULL;
-	}
-
 	m = meta_data_create();
+	s = PyList_Size(l);
 	for (i = 0; i < s; ++i) {
 		const char *string, *keystring;
 		PyObject *key, *value, *item, *tmp;
 		
 		item = PyList_GET_ITEM(l, i);
 		key = PyTuple_GET_ITEM(item, 0);
