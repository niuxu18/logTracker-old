{
	const char *s1, *s2;
	int cmp;
	int l;

	s1 = p1->identifier;
	s2 = p2->identifier;

	/* Compare File Name */
	l = p1->ext_off;
	if (l > p2->ext_off)
		l = p2->ext_off;
	cmp = memcmp(s1, s2, l);
	if (cmp != 0)
		return (cmp);
	if (p1->ext_off < p2->ext_off) {
		s2 += l;
		l = p2->ext_off - p1->ext_off;
		while (l--)
			if (0x20 != *s2++)
				return (0x20
				    - *(const unsigned char *)(s2 - 1));
	} else if (p1->ext_off > p2->ext_off) {
		s1 += l;
		l = p1->ext_off - p2->ext_off;
		while (l--)
			if (0x20 != *s1++)
				return (*(const unsigned char *)(s1 - 1)
				    - 0x20);
	}
	/* Compare File Name Extension */
	if (p1->ext_len == 0 && p2->ext_len == 0)
		return (0);
	if (p1->ext_len == 1 && p2->ext_len == 1)
		return (0);
	if (p1->ext_len <= 1)
		return (-1);
	if (p2->ext_len <= 1)
		return (1);
	l = p1->ext_len;
	if (l > p2->ext_len)
		l = p2->ext_len;
	s1 = p1->identifier + p1->ext_off;
	s2 = p2->identifier + p2->ext_off;
	if (l > 1) {
		cmp = memcmp(s1, s2, l);
		if (cmp != 0)
			return (cmp);
	}
	if (p1->ext_len < p2->ext_len) {
		s2 += l;
		l = p2->ext_len - p1->ext_len;
		while (l--)
			if (0x20 != *s2++)
				return (0x20
				    - *(const unsigned char *)(s2 - 1));
	} else if (p1->ext_len > p2->ext_len) {
		s1 += l;
		l = p1->ext_len - p2->ext_len;
		while (l--)
			if (0x20 != *s1++)
				return (*(const unsigned char *)(s1 - 1)
				    - 0x20);
	}
	/* Compare File Version Number */
	/* No operation. The File Version Number is always one. */

	return (cmp);
}