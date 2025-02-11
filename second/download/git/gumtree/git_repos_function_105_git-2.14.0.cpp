static void parse_commit_header(struct format_commit_context *context)
{
	const char *msg = context->message;
	int i;

	for (i = 0; msg[i]; i++) {
		const char *name;
		int eol;
		for (eol = i; msg[eol] && msg[eol] != '\n'; eol++)
			; /* do nothing */

		if (i == eol) {
			break;
		} else if (skip_prefix(msg + i, "author ", &name)) {
			context->author.off = name - msg;
			context->author.len = msg + eol - name;
		} else if (skip_prefix(msg + i, "committer ", &name)) {
			context->committer.off = name - msg;
			context->committer.len = msg + eol - name;
		}
		i = eol;
	}
	context->message_off = i;
	context->commit_header_parsed = 1;
}