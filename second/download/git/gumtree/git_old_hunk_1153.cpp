		strbuf_release(&stash_sha1);
		return 0;
	}
	strbuf_trim(&stash_sha1);

	child.git_cmd = 1;
	argv_array_push(&child.args, "stash");
	argv_array_push(&child.args, "apply");
	argv_array_push(&child.args, stash_sha1.buf);
	if (!run_command(&child))
		printf(_("Applied autostash."));
	else {
		struct child_process store = CHILD_PROCESS_INIT;

		store.git_cmd = 1;
		argv_array_push(&store.args, "stash");
		argv_array_push(&store.args, "store");
