 	if (args->use_ofs_delta)
 		argv[i++] = "--delta-base-offset";
 	if (args->quiet || !args->progress)
 		argv[i++] = "-q";
 	if (args->progress)
 		argv[i++] = "--progress";
+	if (is_repository_shallow())
+		argv[i++] = "--shallow";
 	po.argv = argv;
 	po.in = -1;
 	po.out = args->stateless_rpc ? -1 : fd;
 	po.git_cmd = 1;
 	if (start_command(&po))
 		die_errno("git pack-objects failed");
