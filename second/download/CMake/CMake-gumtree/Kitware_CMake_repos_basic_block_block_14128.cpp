{
	const char *s;
	int n;
	char *p;
	char *end;
	uint32_t unicode;
#if HAVE_WCRTOMB
	mbstate_t shift_state;

	memset(&shift_state, 0, sizeof(shift_state));
#else
	/* Clear the shift state before starting. */
	wctomb(NULL, L'\0');
#endif
	(void)sc; /* UNUSED */
	/*
	 * Allocate buffer for MBS.
	 * We need this allocation here since it is possible that
	 * as->s is still NULL.
	 */
	if (archive_string_ensure(as, as->length + len + 1) == NULL)
		return (-1);

	s = (const char *)_p;
	p = as->s + as->length;
	end = as->s + as->buffer_length - MB_CUR_MAX -1;
	while ((n = _utf8_to_unicode(&unicode, s, len)) != 0) {
		wchar_t wc;

		if (p >= end) {
			as->length = p - as->s;
			/* Re-allocate buffer for MBS. */
			if (archive_string_ensure(as,
			    as->length + len * 2 + 1) == NULL)
				return (-1);
			p = as->s + as->length;
			end = as->s + as->buffer_length - MB_CUR_MAX -1;
		}

		/*
		 * As libarchive 2.x, translates the UTF-8 characters into
		 * wide-characters in the assumption that WCS is Unicode.
		 */
		if (n < 0) {
			n *= -1;
			wc = L'?';
		} else
			wc = (wchar_t)unicode;

		s += n;
		len -= n;
		/*
		 * Translates the wide-character into the current locale MBS.
		 */
#if HAVE_WCRTOMB
		n = (int)wcrtomb(p, wc, &shift_state);
#else
		n = (int)wctomb(p, wc);
#endif
		if (n == -1)
			return (-1);
		p += n;
	}
	as->length = p - as->s;
	as->s[as->length] = '\0';
	return (0);
}