int normalize_path_copy_len(char *dst, const char *src, int *prefix_len)
{
	char *dst0;
	int i;

	for (i = has_dos_drive_prefix(src); i > 0; i--)
		*dst++ = *src++;
	dst0 = dst;

	if (is_dir_sep(*src)) {
		*dst++ = '/';
		while (is_dir_sep(*src))
			src++;
	}

	for (;;) {
		char c = *src;

		/*
		 * A path component that begins with . could be
		 * special:
		 * (1) "." and ends   -- ignore and terminate.
		 * (2) "./"           -- ignore them, eat slash and continue.
		 * (3) ".." and ends  -- strip one and terminate.
		 * (4) "../"          -- strip one, eat slash and continue.
		 */
		if (c == '.') {
			if (!src[1]) {
				/* (1) */
				src++;
			} else if (is_dir_sep(src[1])) {
				/* (2) */
				src += 2;
				while (is_dir_sep(*src))
					src++;
				continue;
			} else if (src[1] == '.') {
				if (!src[2]) {
					/* (3) */
					src += 2;
					goto up_one;
				} else if (is_dir_sep(src[2])) {
					/* (4) */
					src += 3;
					while (is_dir_sep(*src))
						src++;
					goto up_one;
				}
			}
		}

		/* copy up to the next '/', and eat all '/' */
		while ((c = *src++) != '\0' && !is_dir_sep(c))
			*dst++ = c;
		if (is_dir_sep(c)) {
			*dst++ = '/';
			while (is_dir_sep(c))
				c = *src++;
			src--;
		} else if (!c)
			break;
		continue;

	up_one:
		/*
		 * dst0..dst is prefix portion, and dst[-1] is '/';
		 * go up one level.
		 */
		dst--;	/* go to trailing '/' */
		if (dst <= dst0)
			return -1;
		/* Windows: dst[-1] cannot be backslash anymore */
		while (dst0 < dst && dst[-1] != '/')
			dst--;
		if (prefix_len && *prefix_len > dst - dst0)
			*prefix_len = dst - dst0;
	}
	*dst = '\0';
	return 0;
}