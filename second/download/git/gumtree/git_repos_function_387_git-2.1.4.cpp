static char *do_askpass(const char *cmd, const char *prompt)
{
	struct child_process pass;
	const char *args[3];
	static struct strbuf buffer = STRBUF_INIT;
	int err = 0;

	args[0] = cmd;
	args[1]	= prompt;
	args[2] = NULL;

	memset(&pass, 0, sizeof(pass));
	pass.argv = args;
	pass.out = -1;

	if (start_command(&pass))
		return NULL;

	strbuf_reset(&buffer);
	if (strbuf_read(&buffer, pass.out, 20) < 0)
		err = 1;

	close(pass.out);

	if (finish_command(&pass))
		err = 1;

	if (err) {
		error("unable to read askpass response from '%s'", cmd);
		strbuf_release(&buffer);
		return NULL;
	}

	strbuf_setlen(&buffer, strcspn(buffer.buf, "\r\n"));

	return buffer.buf;
}