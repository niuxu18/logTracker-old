static int push_git(struct discovery *heads, int nr_spec, char **specs)
{
	struct rpc_state rpc;
	int i, err;
	struct argv_array args;
	struct string_list_item *cas_option;

	argv_array_init(&args);
	argv_array_pushl(&args, "send-pack", "--stateless-rpc", "--helper-status",
			 NULL);

	if (options.thin)
		argv_array_push(&args, "--thin");
	if (options.dry_run)
		argv_array_push(&args, "--dry-run");
	if (options.verbosity == 0)
		argv_array_push(&args, "--quiet");
	else if (options.verbosity > 1)
		argv_array_push(&args, "--verbose");
	argv_array_push(&args, options.progress ? "--progress" : "--no-progress");
	for_each_string_list_item(cas_option, &cas_options)
		argv_array_push(&args, cas_option->string);
	argv_array_push(&args, url.buf);
	for (i = 0; i < nr_spec; i++)
		argv_array_push(&args, specs[i]);

	memset(&rpc, 0, sizeof(rpc));
	rpc.service_name = "git-receive-pack",
	rpc.argv = args.argv;

	err = rpc_service(&rpc, heads);
	if (rpc.result.len)
		write_or_die(1, rpc.result.buf, rpc.result.len);
	strbuf_release(&rpc.result);
	argv_array_clear(&args);
	return err;
}