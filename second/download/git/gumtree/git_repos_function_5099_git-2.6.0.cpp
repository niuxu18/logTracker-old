static int read_one_header_line(struct strbuf *line, FILE *in)
{
	/* Get the first part of the line. */
	if (strbuf_getline(line, in, '\n'))
		return 0;

	/*
	 * Is it an empty line or not a valid rfc2822 header?
	 * If so, stop here, and return false ("not a header")
	 */
	strbuf_rtrim(line);
	if (!line->len || !is_rfc2822_header(line)) {
		/* Re-add the newline */
		strbuf_addch(line, '\n');
		return 0;
	}

	/*
	 * Now we need to eat all the continuation lines..
	 * Yuck, 2822 header "folding"
	 */
	for (;;) {
		int peek;
		struct strbuf continuation = STRBUF_INIT;

		peek = fgetc(in); ungetc(peek, in);
		if (peek != ' ' && peek != '\t')
			break;
		if (strbuf_getline(&continuation, in, '\n'))
			break;
		continuation.buf[0] = ' ';
		strbuf_rtrim(&continuation);
		strbuf_addbuf(line, &continuation);
	}

	return 1;
}