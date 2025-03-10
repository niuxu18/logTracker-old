	else
		boc += 2;
	eoc = push_cert->buf + parse_signature(push_cert->buf, push_cert->len);

	while (boc < eoc) {
		const char *eol = memchr(boc, '\n', eoc - boc);
		tail = queue_command(tail, boc, eol ? eol - boc : eoc - eol);
		boc = eol ? eol + 1 : eoc;
	}
}

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
