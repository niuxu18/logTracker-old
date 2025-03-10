 	return 0;
 }
 
 static int push_check(int argc, const char **argv, const char *prefix)
 {
 	struct remote *remote;
+	const char *superproject_head;
+	char *head;
+	int detached_head = 0;
+	struct object_id head_oid;
 
-	if (argc < 2)
-		die("submodule--helper push-check requires at least 1 argument");
+	if (argc < 3)
+		die("submodule--helper push-check requires at least 2 arguments");
+
+	/*
+	 * superproject's resolved head ref.
+	 * if HEAD then the superproject is in a detached head state, otherwise
+	 * it will be the resolved head ref.
+	 */
+	superproject_head = argv[1];
+	argv++;
+	argc--;
+	/* Get the submodule's head ref and determine if it is detached */
+	head = resolve_refdup("HEAD", 0, head_oid.hash, NULL);
+	if (!head)
+		die(_("Failed to resolve HEAD as a valid ref."));
+	if (!strcmp(head, "HEAD"))
+		detached_head = 1;
 
 	/*
 	 * The remote must be configured.
 	 * This is to avoid pushing to the exact same URL as the parent.
 	 */
 	remote = pushremote_get(argv[1]);
