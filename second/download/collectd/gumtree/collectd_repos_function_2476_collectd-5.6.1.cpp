int main (int argc, char **argv)
{
	const char *configfile = CONFIGFILE;
	int test_config  = 0;
	int test_readall = 0;
	const char *basedir;
#if COLLECT_DAEMON
	pid_t pid;
	int daemonize    = 1;
#endif
	int exit_status = 0;

	/* read options */
	while (1)
	{
		int c;

		c = getopt (argc, argv, "htTC:"
#if COLLECT_DAEMON
				"fP:"
#endif
		);

		if (c == -1)
			break;

		switch (c)
		{
			case 'C':
				configfile = optarg;
				break;
			case 't':
				test_config = 1;
				break;
			case 'T':
				test_readall = 1;
				global_option_set ("ReadThreads", "-1", 1);
#if COLLECT_DAEMON
				daemonize = 0;
#endif /* COLLECT_DAEMON */
				break;
#if COLLECT_DAEMON
			case 'P':
				global_option_set ("PIDFile", optarg, 1);
				break;
			case 'f':
				daemonize = 0;
				break;
#endif /* COLLECT_DAEMON */
			case 'h':
				exit_usage (0);
				break;
			default:
				exit_usage (1);
		} /* switch (c) */
	} /* while (1) */

	if (optind < argc)
		exit_usage (1);

	plugin_init_ctx ();

	/*
	 * Read options from the config file, the environment and the command
	 * line (in that order, with later options overwriting previous ones in
	 * general).
	 * Also, this will automatically load modules.
	 */
	if (cf_read (configfile))
	{
		fprintf (stderr, "Error: Reading the config file failed!\n"
				"Read the syslog for details.\n");
		return (1);
	}

	/*
	 * Change directory. We do this _after_ reading the config and loading
	 * modules to relative paths work as expected.
	 */
	if ((basedir = global_option_get ("BaseDir")) == NULL)
	{
		fprintf (stderr, "Don't have a basedir to use. This should not happen. Ever.");
		return (1);
	}
	else if (change_basedir (basedir))
	{
		fprintf (stderr, "Error: Unable to change to directory `%s'.\n", basedir);
		return (1);
	}

	/*
	 * Set global variables or, if that failes, exit. We cannot run with
	 * them being uninitialized. If nothing is configured, then defaults
	 * are being used. So this means that the user has actually done
	 * something wrong.
	 */
	if (init_global_variables () != 0)
		return (1);

	if (test_config)
		return (0);

#if COLLECT_DAEMON
	/*
	 * fork off child
	 */
	struct sigaction sig_chld_action = {
		.sa_handler = SIG_IGN
	};

	sigaction (SIGCHLD, &sig_chld_action, NULL);

    /*
     * Only daemonize if we're not being supervised
     * by upstart or systemd (when using Linux).
     */
	if (daemonize
#ifdef KERNEL_LINUX
	    && notify_upstart() == 0 && notify_systemd() == 0
#endif
	)
	{
		int status;

		if ((pid = fork ()) == -1)
		{
			/* error */
			char errbuf[1024];
			fprintf (stderr, "fork: %s",
					sstrerror (errno, errbuf,
						sizeof (errbuf)));
			return (1);
		}
		else if (pid != 0)
		{
			/* parent */
			/* printf ("Running (PID %i)\n", pid); */
			return (0);
		}

		/* Detach from session */
		setsid ();

		/* Write pidfile */
		if (pidfile_create ())
			exit (2);

		/* close standard descriptors */
		close (2);
		close (1);
		close (0);

		status = open ("/dev/null", O_RDWR);
		if (status != 0)
		{
			ERROR ("Error: Could not connect `STDIN' to `/dev/null' (status %d)", status);
			return (1);
		}

		status = dup (0);
		if (status != 1)
		{
			ERROR ("Error: Could not connect `STDOUT' to `/dev/null' (status %d)", status);
			return (1);
		}

		status = dup (0);
		if (status != 2)
		{
			ERROR ("Error: Could not connect `STDERR' to `/dev/null', (status %d)", status);
			return (1);
		}
	} /* if (daemonize) */
#endif /* COLLECT_DAEMON */

	struct sigaction sig_pipe_action = {
		.sa_handler = SIG_IGN
	};

	sigaction (SIGPIPE, &sig_pipe_action, NULL);

	/*
	 * install signal handlers
	 */
	struct sigaction sig_int_action = {
		.sa_handler = sig_int_handler
	};

	if (0 != sigaction (SIGINT, &sig_int_action, NULL)) {
		char errbuf[1024];
		ERROR ("Error: Failed to install a signal handler for signal INT: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (1);
	}

	struct sigaction sig_term_action = {
		.sa_handler = sig_term_handler
	};

	if (0 != sigaction (SIGTERM, &sig_term_action, NULL)) {
		char errbuf[1024];
		ERROR ("Error: Failed to install a signal handler for signal TERM: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (1);
	}

	struct sigaction sig_usr1_action = {
		.sa_handler = sig_usr1_handler
	};

	if (0 != sigaction (SIGUSR1, &sig_usr1_action, NULL)) {
		char errbuf[1024];
		ERROR ("Error: Failed to install a signal handler for signal USR1: %s",
				sstrerror (errno, errbuf, sizeof (errbuf)));
		return (1);
	}

	/*
	 * run the actual loops
	 */
	if (do_init () != 0)
	{
		ERROR ("Error: one or more plugin init callbacks failed.");
		exit_status = 1;
	}

	if (test_readall)
	{
		if (plugin_read_all_once () != 0)
		{
			ERROR ("Error: one or more plugin read callbacks failed.");
			exit_status = 1;
		}
	}
	else
	{
		INFO ("Initialization complete, entering read-loop.");
		do_loop ();
	}

	/* close syslog */
	INFO ("Exiting normally.");

	if (do_shutdown () != 0)
	{
		ERROR ("Error: one or more plugin shutdown callbacks failed.");
		exit_status = 1;
	}

#if COLLECT_DAEMON
	if (daemonize)
		pidfile_remove ();
#endif /* COLLECT_DAEMON */

	return (exit_status);
}