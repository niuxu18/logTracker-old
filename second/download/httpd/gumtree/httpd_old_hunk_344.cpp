
    if (parse_url(apr_pstrdup(cntxt, opt->argv[opt->ind++]))) {
	fprintf(stderr, "%s: invalid URL\n", argv[0]);
	usage(argv[0]);
    }


    if ((heartbeatres) && (requests > 150)) {
	heartbeatres = requests / 10;	/* Print line every 10% of requests */
	if (heartbeatres < 100)
	    heartbeatres = 100;	/* but never more often than once every 100
				 * connections. */
