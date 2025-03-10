   Py_XDECREF(tmp);
 
   self->interval = interval;
   return 0;
 }
 
-static meta_data_t *cpy_build_meta(PyObject *meta) {
+static int cpy_build_meta_generic(PyObject *meta,
+                                  cpy_build_meta_handler_t *meta_func,
+                                  void *m) {
   int s;
-  meta_data_t *m = NULL;
   PyObject *l;
 
   if ((meta == NULL) || (meta == Py_None))
-    return NULL;
+    return -1;
 
   l = PyDict_Items(meta); /* New reference. */
   if (!l) {
     cpy_log_exception("building meta data");
-    return NULL;
+    return -1;
   }
   s = PyList_Size(l);
   if (s <= 0) {
     Py_XDECREF(l);
-    return NULL;
+    return -1;
   }
 
-  m = meta_data_create();
   for (int i = 0; i < s; ++i) {
     const char *string, *keystring;
     PyObject *key, *value, *item, *tmp;
 
     item = PyList_GET_ITEM(l, i);
     key = PyTuple_GET_ITEM(item, 0);
