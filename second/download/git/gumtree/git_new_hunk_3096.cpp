	else if (need_out)
		fhout = dup(fdout[1]);
	else if (cmd->out > 1)
		fhout = dup(cmd->out);

	if (cmd->git_cmd)
		cmd->argv = prepare_git_cmd(&nargv, cmd->argv);
	else if (cmd->use_shell)
		cmd->argv = prepare_shell_cmd(&nargv, cmd->argv);

	cmd->pid = mingw_spawnvpe(cmd->argv[0], cmd->argv, (char**) cmd->env,
			cmd->dir, fhin, fhout, fherr);
	failed_errno = errno;
	if (cmd->pid < 0 && (!cmd->silent_exec_failure || errno != ENOENT))
		error("cannot spawn %s: %s", cmd->argv[0], strerror(errno));
	if (cmd->clean_on_exit && cmd->pid >= 0)
		mark_child_for_cleanup(cmd->pid);

	argv_array_clear(&nargv);
	cmd->argv = sargv;
	if (fhin != 0)
		close(fhin);
	if (fhout != 1)
		close(fhout);
	if (fherr != 2)
