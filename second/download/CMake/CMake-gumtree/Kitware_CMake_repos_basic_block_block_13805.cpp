{
	size_t r;
	int ret_val = 0;
	/*
	 * No single byte will be more than one wide character,
	 * so this length estimate will always be big enough.
	 */
	size_t wcs_length = len;
	size_t mbs_length = len;
	const char *mbs = p;
	wchar_t *wcs;
#if HAVE_MBRTOWC
	mbstate_t shift_state;

	memset(&shift_state, 0, sizeof(shift_state));
#endif
	if (NULL == archive_wstring_ensure(dest, dest->length + wcs_length + 1))
		return (-1);
	wcs = dest->s + dest->length;
	/*
	 * We cannot use mbsrtowcs/mbstowcs here because those may convert
	 * extra MBS when strlen(p) > len and one wide character consists of
	 * multi bytes.
	 */
	while (*mbs && mbs_length > 0) {
		if (wcs_length == 0) {
			dest->length = wcs - dest->s;
			dest->s[dest->length] = L'\0';
			wcs_length = mbs_length;
			if (NULL == archive_wstring_ensure(dest,
			    dest->length + wcs_length + 1))
				return (-1);
			wcs = dest->s + dest->length;
		}
#if HAVE_MBRTOWC
		r = mbrtowc(wcs, mbs, wcs_length, &shift_state);
#else
		r = mbtowc(wcs, mbs, wcs_length);
#endif
		if (r == (size_t)-1 || r == (size_t)-2) {
			ret_val = -1;
			if (errno == EILSEQ) {
				++mbs;
				--mbs_length;
				continue;
			} else
				break;
		}
		if (r == 0 || r > mbs_length)
			break;
		wcs++;
		wcs_length--;
		mbs += r;
		mbs_length -= r;
	}
	dest->length = wcs - dest->s;
	dest->s[dest->length] = L'\0';
	return (ret_val);
}