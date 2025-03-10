static int run_update_hook(struct command *cmd)
{
	const char *argv[5];
	struct child_process proc = CHILD_PROCESS_INIT;
	int code;

	argv[0] = find_hook("update");
	if (!argv[0])
		return 0;

	argv[1] = cmd->ref_name;
	argv[2] = sha1_to_hex(cmd->old_sha1);
	argv[3] = sha1_to_hex(cmd->new_sha1);
	argv[4] = NULL;

	proc.no_stdin = 1;
	proc.stdout_to_stderr = 1;
	proc.err = use_sideband ? -1 : 0;
	proc.argv = argv;
	proc.env = tmp_objdir_env(tmp_objdir);

	code = start_command(&proc);
	if (code)
		return code;
	if (use_sideband)
		copy_to_sideband(proc.err, -1, NULL);
	return finish_command(&proc);
}