fprintf (stderr, "Usage: %s <-s socket> <-n value_spec> <-H hostname> [options]\n"
			"\n"
			"Valid options are:\n"
			"  -s <socket>    Path to collectd's UNIX-socket.\n"
			"  -n <v_spec>    Value specification to get from collectd.\n"
			"                 Format: `plugin-instance/type-instance'\n"
			"  -d <ds>        Select the DS to examine. May be repeated to examine multiple\n"
			"                 DSes. By default all DSes are used.\n"
			"  -g <consol>    Method to use to consolidate several DSes.\n"
			"                 Valid arguments are `none', `average' and `sum'\n"
			"  -H <host>      Hostname to query the values for.\n"
			"  -c <range>     Critical range\n"
			"  -w <range>     Warning range\n"
			"\n"
			"Consolidation functions:\n"
			"  none:          Apply the warning- and critical-ranges to each data-source\n"
			"                 individually.\n"
			"  average:       Calculate the average of all matching DSes and apply the\n"
			"                 warning- and critical-ranges to the calculated average.\n"
			"  sum:           Apply the ranges to the sum of all DSes.\n"
			"\n", name);