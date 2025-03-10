const char *ident_default_email(void)
{
	if (!git_default_email.len) {
		const char *email = getenv("EMAIL");

		if (email && email[0]) {
			strbuf_addstr(&git_default_email, email);
			committer_ident_explicitly_given |= IDENT_MAIL_GIVEN;
			author_ident_explicitly_given |= IDENT_MAIL_GIVEN;
		} else
			copy_email(xgetpwuid_self(), &git_default_email);
		strbuf_trim(&git_default_email);
	}
	return git_default_email.buf;
}