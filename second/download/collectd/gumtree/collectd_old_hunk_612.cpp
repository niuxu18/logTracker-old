 **/

/* Some python versions don't include this by default. */

#include <longintrepr.h>

/* These two macros are basically Py_BEGIN_ALLOW_THREADS and Py_BEGIN_ALLOW_THREADS
 * from the other direction. If a Python thread calls a C function
 * Py_BEGIN_ALLOW_THREADS is used to allow other python threads to run because
 * we don't intend to call any Python functions.
 *
 * These two macros are used whenever a C thread intends to call some Python
 * function, usually because some registered callback was triggered.
 * Just like Py_BEGIN_ALLOW_THREADS it opens a block so these macros have to be
 * used in pairs. They acquire the GIL, create a new Python thread state and swap
 * the current thread state with the new one. This means this thread is now allowed
 * to execute Python code. */

#define CPY_LOCK_THREADS {\
	PyGILState_STATE gil_state;\
	gil_state = PyGILState_Ensure();

#define CPY_RETURN_FROM_THREADS \
	PyGILState_Release(gil_state);\
	return

#define CPY_RELEASE_THREADS \
	PyGILState_Release(gil_state);\
}

/* This macro is a shortcut for calls like
 * x = PyObject_Repr(x);
 * This can't be done like this example because this would leak
 * a reference the the original x and crash in case of x == NULL.
 * This calling syntax is less than elegant but it works, saves
 * a lot of lines and avoids potential refcount errors. */

#define CPY_SUBSTITUTE(func, a, ...) do {\
	if ((a) != NULL) {\
		PyObject *__tmp = (a);\
		(a) = func(__VA_ARGS__);\
		Py_DECREF(__tmp);\
	}\
} while(0)

/* Python3 compatibility layer. To keep the actual code as clean as possible
 * do a lot of defines here. */

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K

#define PyInt_FromLong PyLong_FromLong
#define CPY_INIT_TYPE         PyVarObject_HEAD_INIT(NULL, 0)
#define IS_BYTES_OR_UNICODE(o) (PyUnicode_Check(o) || PyBytes_Check(o))
#define CPY_STRCAT_AND_DEL(a, b) do {\
	CPY_STRCAT((a), (b));\
	Py_XDECREF((b));\
} while (0)
static inline void CPY_STRCAT(PyObject **a, PyObject *b) {
	PyObject *ret;

	if (!a || !*a)
		return;

	ret = PyUnicode_Concat(*a, b);
	Py_DECREF(*a);
	*a = ret;
}

#else

#define CPY_INIT_TYPE         PyObject_HEAD_INIT(NULL) 0,
#define IS_BYTES_OR_UNICODE(o) (PyUnicode_Check(o) || PyString_Check(o))
#define CPY_STRCAT_AND_DEL PyString_ConcatAndDel
#define CPY_STRCAT PyString_Concat

#endif

static inline const char *cpy_unicode_or_bytes_to_string(PyObject **o) {
	if (PyUnicode_Check(*o)) {
		PyObject *tmp;
		tmp = PyUnicode_AsEncodedString(*o, NULL, NULL); /* New reference. */
		if (tmp == NULL)
			return NULL;
		Py_DECREF(*o);
		*o = tmp;
	}
#ifdef IS_PY3K
	return PyBytes_AsString(*o);
#else
	return PyString_AsString(*o);
#endif
}

static inline PyObject *cpy_string_to_unicode_or_bytes(const char *buf) {
#ifdef IS_PY3K
/* Python3 preferrs unicode */
	PyObject *ret;
	ret = PyUnicode_Decode(buf, strlen(buf), NULL, NULL);
	if (ret != NULL)
		return ret;
	PyErr_Clear();
	return PyBytes_FromString(buf);
#else
	return PyString_FromString(buf);
#endif
}

void cpy_log_exception(const char *context);

/* Python object declarations. */

typedef struct {
	PyObject_HEAD        /* No semicolon! */
	PyObject *parent;    /* Config */
	PyObject *key;       /* String */
	PyObject *values;    /* Sequence */
	PyObject *children;  /* Sequence */
} Config;
extern PyTypeObject ConfigType;

typedef struct {
	PyObject_HEAD        /* No semicolon! */
	double time;
	char host[DATA_MAX_NAME_LEN];
	char plugin[DATA_MAX_NAME_LEN];
	char plugin_instance[DATA_MAX_NAME_LEN];
	char type[DATA_MAX_NAME_LEN];
	char type_instance[DATA_MAX_NAME_LEN];
} PluginData;
extern PyTypeObject PluginDataType;
#define PluginData_New() PyObject_CallFunctionObjArgs((PyObject *) &PluginDataType, (void *) 0)

typedef struct {
	PluginData data;
	PyObject *values;    /* Sequence */
	PyObject *meta;      /* dict */
	double interval;
} Values;
extern PyTypeObject ValuesType;
#define Values_New() PyObject_CallFunctionObjArgs((PyObject *) &ValuesType, (void *) 0)

typedef struct {
	PluginData data;
	int severity;
	char message[NOTIF_MAX_MSG_LEN];
} Notification;
extern PyTypeObject NotificationType;
#define Notification_New() PyObject_CallFunctionObjArgs((PyObject *) &NotificationType, (void *) 0)

typedef PyLongObject Signed;
extern PyTypeObject SignedType;

typedef PyLongObject Unsigned;
extern PyTypeObject UnsignedType;
