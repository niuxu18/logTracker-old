int color_parse_mem(const char *value, int value_len, char *dst)
{
	const char *ptr = value;
	int len = value_len;
	char *end = dst + COLOR_MAXLEN;
	unsigned int attr = 0;
	struct color fg = { COLOR_UNSPECIFIED };
	struct color bg = { COLOR_UNSPECIFIED };

	if (!strncasecmp(value, "reset", len)) {
		xsnprintf(dst, end - dst, GIT_COLOR_RESET);
		return 0;
	}

	/* [fg [bg]] [attr]... */
	while (len > 0) {
		const char *word = ptr;
		struct color c = { COLOR_UNSPECIFIED };
		int val, wordlen = 0;

		while (len > 0 && !isspace(word[wordlen])) {
			wordlen++;
			len--;
		}

		ptr = word + wordlen;
		while (len > 0 && isspace(*ptr)) {
			ptr++;
			len--;
		}

		if (!parse_color(&c, word, wordlen)) {
			if (fg.type == COLOR_UNSPECIFIED) {
				fg = c;
				continue;
			}
			if (bg.type == COLOR_UNSPECIFIED) {
				bg = c;
				continue;
			}
			goto bad;
		}
		val = parse_attr(word, wordlen);
		if (0 <= val)
			attr |= (1 << val);
		else
			goto bad;
	}

#undef OUT
#define OUT(x) do { \
	if (dst == end) \
		die("BUG: color parsing ran out of space"); \
	*dst++ = (x); \
} while(0)

	if (attr || !color_empty(&fg) || !color_empty(&bg)) {
		int sep = 0;
		int i;

		OUT('\033');
		OUT('[');

		for (i = 0; attr; i++) {
			unsigned bit = (1 << i);
			if (!(attr & bit))
				continue;
			attr &= ~bit;
			if (sep++)
				OUT(';');
			dst += xsnprintf(dst, end - dst, "%d", i);
		}
		if (!color_empty(&fg)) {
			if (sep++)
				OUT(';');
			/* foreground colors are all in the 3x range */
			dst = color_output(dst, end - dst, &fg, '3');
		}
		if (!color_empty(&bg)) {
			if (sep++)
				OUT(';');
			/* background colors are all in the 4x range */
			dst = color_output(dst, end - dst, &bg, '4');
		}
		OUT('m');
	}
	OUT(0);
	return 0;
bad:
	return error(_("invalid color value: %.*s"), value_len, value);
#undef OUT
}