	    fprintf(out, "starttime\tseconds\tctime\tdtime\tttime\twait\n");
	    for (i = 0; i < requests; i++) {
                apr_time_t diff = stats[i].time - stats[i].ctime;

		sttime = stats[i].starttime;
		(void) apr_ctime(tmstring, sttime);
		fprintf(out, "%s\t%" APR_TIME_T_FMT "\t%" APR_TIME_T_FMT "\t%" APR_TIME_T_FMT "\t%" APR_TIME_T_FMT "\t%" APR_TIME_T_FMT "\n",
			tmstring,
			sttime,
			stats[i].ctime,
			diff,
			stats[i].time,
