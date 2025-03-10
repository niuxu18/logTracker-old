     fprintf (fh, "PluginInstance: %s\n", n->plugin_instance);
   if (strlen (n->type) > 0)
     fprintf (fh, "Type: %s\n", n->type);
   if (strlen (n->type_instance) > 0)
     fprintf (fh, "TypeInstance: %s\n", n->type_instance);
 
+  for (meta = n->meta; meta != NULL; meta = meta->next)
+  {
+    if (meta->type == NM_TYPE_STRING)
+      fprintf (fh, "%s: %s\n", meta->name, meta->value_string);
+    else if (meta->type == NM_TYPE_SIGNED_INT)
+      fprintf (fh, "%s: %"PRIi64"\n", meta->name, meta->value_signed_int);
+    else if (meta->type == NM_TYPE_UNSIGNED_INT)
+      fprintf (fh, "%s: %"PRIu64"\n", meta->name, meta->value_unsigned_int);
+    else if (meta->type == NM_TYPE_DOUBLE)
+      fprintf (fh, "%s: %e\n", meta->name, meta->value_double);
+    else if (meta->type == NM_TYPE_BOOLEAN)
+      fprintf (fh, "%s: %s\n", meta->name,
+	  meta->value_boolean ? "true" : "false");
+  }
+
   fprintf (fh, "\n%s\n", n->message);
 
   fflush (fh);
   fclose (fh);
 
   waitpid (pid, &status, 0);
 
   DEBUG ("exec plugin: Child %i exited with status %i.",
       pid, status);
 
+  plugin_notification_meta_free (n);
   sfree (arg);
   pthread_exit ((void *) 0);
   return (NULL);
 } /* void *exec_notification_one }}} */
 
 static int exec_init (void) /* {{{ */
