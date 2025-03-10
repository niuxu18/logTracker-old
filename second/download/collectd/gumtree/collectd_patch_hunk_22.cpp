  *  If `dir' is NULL the compiled in default `PLUGINDIR' is used.
  */
 void plugin_set_dir (const char *dir);
 
 /*
  * NAME
- *  plugin_count
- *
- * DESCRIPTION
- *  trivial
- *
- * RETURN VALUE
- *  The number of currently loaded plugins
- */
-int plugin_count (void);
-
-/*
- * NAME
- *  plugin_exists
- *
- * DESCRIPTION
- *  trivial
- *
- * ARGUMENTS
- *  `type'      Name of the plugin.
- *
- * RETURN VALUE
- *  Returns non-zero if a plugin with the name $type is found and zero
- *  otherwise.
- */
-int plugin_exists (char *type);
-
-/*
- * NAME
  *  plugin_load
  *
  * DESCRIPTION
  *  Searches the current `plugindir' (see `plugin_set_dir') for the plugin
  *  named $type and loads it. Afterwards the plugin's `module_register'
  *  function is called, which then calls `plugin_register' to register callback
  *  functions.
  *
  * ARGUMENTS
- *  `type'      Name of the plugin to load.
+ *  `name'      Name of the plugin to load.
+ *  `mr'        Types of functions to request from the plugin.
  *
  * RETURN VALUE
  *  Returns zero upon success, a value greater than zero if no plugin was found
  *  and a value below zero if an error occurs.
  *
  * NOTES
  *  No attempt is made to re-load an already loaded module.
  */
-int  plugin_load (const char *type);
+int plugin_load (const char *name);
 
-int  plugin_load_all (char *dir);
 void plugin_init_all (void);
 void plugin_read_all (const int *loop);
-
 void plugin_shutdown_all (void);
 
-void plugin_register (char *type,
-		void (*init) (void),
-		void (*read) (void),
-		void (*write) (char *, char *, char *));
-
-int plugin_register_shutdown (char *, void (*) (void));
+/*
+ * The `plugin_register_*' functions are used to make `config', `init',
+ * `read', `write' and `shutdown' functions known to the plugin
+ * infrastructure. Also, the data-formats are made public like this.
+ */
+int plugin_register_config (const char *name,
+		int (*callback) (const char *key, const char *val),
+		const char **keys, int keys_num);
+int plugin_register_init (const char *name,
+		int (*callback) (void));
+int plugin_register_read (const char *name,
+		int (*callback) (void));
+int plugin_register_write (const char *name,
+		int (*callback) (const data_set_t *ds, const value_list_t *vl));
+int plugin_register_shutdown (char *name,
+		int (*callback) (void));
+int plugin_register_data_set (const data_set_t *ds);
+int plugin_register_log (char *name,
+		void (*callback) (int, const char *));
+
+int plugin_unregister_config (const char *name);
+int plugin_unregister_init (const char *name);
+int plugin_unregister_read (const char *name);
+int plugin_unregister_write (const char *name);
+int plugin_unregister_shutdown (const char *name);
+int plugin_unregister_data_set (const char *name);
+int plugin_unregister_log (const char *name);
 
 /*
  * NAME
- *  plugin_write
+ *  plugin_dispatch_values
  *
  * DESCRIPTION
- *  Searches the plugin for `type' in the plugin-list. If found, and a `write'
- *  function is registered, it's called. If either the plugin is not found or
- *  the plugin doesn't provide a `write' function this function will return
- *  without further notice.
+ *  This function is called by reading processes with the values they've
+ *  aquired. The function fetches the data-set definition (that has been
+ *  registered using `plugin_register_data_set') and calls _all_ registered
+ *  write-functions.
  *
  * ARGUMENTS
- *  `host'      Host(name) from which the data originates.
- *  `type'      Name of the plugin.
- *  `inst'      Instance (passed to the plugin's `write' function.
- *  `val'       Values for the RRD files. Also passed to the plugin.
- */
-void plugin_write    (char *host, char *type, char *inst, char *val);
-
-void plugin_submit   (char *type, char *inst, char *val);
-
+ *  `name'      Name/type of the data-set that describe the values in `vl'.
+ *  `vl'        Value list of the values that have been read by a `read'
+ *              function.
+ */
+int plugin_dispatch_values (const char *name, value_list_t *vl);
+
+void plugin_log (int level, const char *format, ...);
+#define ERROR(...)   plugin_log (LOG_ERR,     __VA_ARGS__)
+#define WARNING(...) plugin_log (LOG_WARNING, __VA_ARGS__)
+#define NOTICE(...)  plugin_log (LOG_NOTICE,  __VA_ARGS__)
+#define INFO(...)    plugin_log (LOG_INFO,    __VA_ARGS__)
+#define DEBUG(...)   plugin_log (LOG_DEBUG,   __VA_ARGS__)
 
+/* TODO: Move plugin_{complain,relief} into `utils_complain.[ch]'. -octo */
 void plugin_complain (int level, complain_t *c, const char *format, ...);
 void plugin_relief (int level, complain_t *c, const char *format, ...);
 
+const data_set_t *plugin_get_ds (const char *name);
+
 #endif /* PLUGIN_H */
