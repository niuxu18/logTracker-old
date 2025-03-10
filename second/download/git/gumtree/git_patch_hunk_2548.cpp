 					&cb);
 		free(ref);
 	}
 	return status;
 }
 
+static int cmd_reflog_exists(int argc, const char **argv, const char *prefix)
+{
+	int i, start = 0;
+
+	for (i = 1; i < argc; i++) {
+		const char *arg = argv[i];
+		if (!strcmp(arg, "--")) {
+			i++;
+			break;
+		}
+		else if (arg[0] == '-')
+			usage(reflog_exists_usage);
+		else
+			break;
+	}
+
+	start = i;
+
+	if (argc - start != 1)
+		usage(reflog_exists_usage);
+
+	if (check_refname_format(argv[start], REFNAME_ALLOW_ONELEVEL))
+		die("invalid ref format: %s", argv[start]);
+	return !reflog_exists(argv[start]);
+}
+
 /*
  * main "reflog"
  */
 
 static const char reflog_usage[] =
-"git reflog [ show | expire | delete ]";
+"git reflog [ show | expire | delete | exists ]";
 
 int cmd_reflog(int argc, const char **argv, const char *prefix)
 {
 	if (argc > 1 && !strcmp(argv[1], "-h"))
 		usage(reflog_usage);
 
