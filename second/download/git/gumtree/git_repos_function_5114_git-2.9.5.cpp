static struct command *read_head_info(struct sha1_array *shallow)
{
	struct command *commands = NULL;
	struct command **p = &commands;
	for (;;) {
		char *line;
		int len, linelen;

		line = packet_read_line(0, &len);
		if (!line)
			break;

		if (len == 48 && starts_with(line, "shallow ")) {
			unsigned char sha1[20];
			if (get_sha1_hex(line + 8, sha1))
				die("protocol error: expected shallow sha, got '%s'",
				    line + 8);
			sha1_array_append(shallow, sha1);
			continue;
		}

		linelen = strlen(line);
		if (linelen < len) {
			const char *feature_list = line + linelen + 1;
			if (parse_feature_request(feature_list, "report-status"))
				report_status = 1;
			if (parse_feature_request(feature_list, "side-band-64k"))
				use_sideband = LARGE_PACKET_MAX;
			if (parse_feature_request(feature_list, "quiet"))
				quiet = 1;
			if (advertise_atomic_push
			    && parse_feature_request(feature_list, "atomic"))
				use_atomic = 1;
		}

		if (!strcmp(line, "push-cert")) {
			int true_flush = 0;
			char certbuf[1024];

			for (;;) {
				len = packet_read(0, NULL, NULL,
						  certbuf, sizeof(certbuf), 0);
				if (!len) {
					true_flush = 1;
					break;
				}
				if (!strcmp(certbuf, "push-cert-end\n"))
					break; /* end of cert */
				strbuf_addstr(&push_cert, certbuf);
			}

			if (true_flush)
				break;
			continue;
		}

		p = queue_command(p, line, linelen);
	}

	if (push_cert.len)
		queue_commands_from_cert(p, &push_cert);

	return commands;
}