static void exit_usage (const char *name)
{
	printf ("Usage: %s <options> [-- <collectd options>]\n"

			"\nAvailable options:\n"
			"  -h         Display this help and exit.\n"
			"  -c <path>  Path to the collectd binary.\n"
			"  -P <file>  PID-file.\n"

			"\nFor <collectd options> see collectd.conf(5).\n"

			"\n"PACKAGE_NAME" "PACKAGE_VERSION", http://collectd.org/\n"
			"by Florian octo Forster <octo@collectd.org>\n"
			"for contributions see `AUTHORS'\n", name);
	exit (0);
}