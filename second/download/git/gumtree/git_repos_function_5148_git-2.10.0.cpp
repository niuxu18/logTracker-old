static void set_connectivity_errors(struct command *commands,
				    struct shallow_info *si)
{
	struct command *cmd;

	for (cmd = commands; cmd; cmd = cmd->next) {
		struct command *singleton = cmd;
		if (shallow_update && si->shallow_ref[cmd->index])
			/* to be checked in update_shallow_ref() */
			continue;
		if (!check_connected(command_singleton_iterator, &singleton,
				     NULL))
			continue;
		cmd->error_string = "missing necessary objects";
	}
}