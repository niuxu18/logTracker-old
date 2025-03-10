{
	static struct strbuf ident = STRBUF_INIT;
	int strict = (flag & IDENT_STRICT);
	int want_date = !(flag & IDENT_NO_DATE);
	int want_name = !(flag & IDENT_NO_NAME);

	if (want_name && !name)
		name = ident_default_name();
	if (!email)
		email = ident_default_email();

	if (want_name && !*name) {
		struct passwd *pw;

		if (strict) {
			if (name == git_default_name.buf)
				fputs(env_hint, stderr);
			die("empty ident name (for <%s>) not allowed", email);
		}
		pw = xgetpwuid_self(NULL);
		name = pw->pw_name;
	}

	if (want_name && strict &&
	    name == git_default_name.buf && default_name_is_bogus) {
		fputs(env_hint, stderr);
		die("unable to auto-detect name (got '%s')", name);
	}

	if (strict && email == git_default_email.buf && default_email_is_bogus) {
		fputs(env_hint, stderr);
		die("unable to auto-detect email address (got '%s')", email);
	}

	strbuf_reset(&ident);
	if (want_name) {
		strbuf_addstr_without_crud(&ident, name);
		strbuf_addstr(&ident, " <");
