	} else
		chmod(final_index_name, 0444);

	if (!from_stdin) {
		printf("%s\n", sha1_to_hex(sha1));
	} else {
		char buf[48];
		int len = snprintf(buf, sizeof(buf), "%s\t%s\n",
				   report, sha1_to_hex(sha1));
		write_or_die(1, buf, len);

		/*
		 * Let's just mimic git-unpack-objects here and write
		 * the last part of the input buffer to stdout.
		 */
		while (input_len) {
