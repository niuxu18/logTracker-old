static void msort_with_tmp(void *b, size_t n, size_t s,
			   int (*cmp)(const void *, const void *),
			   char *t)
{
	char *tmp;
	char *b1, *b2;
	size_t n1, n2;

	if (n <= 1)
		return;

	n1 = n / 2;
	n2 = n - n1;
	b1 = b;
	b2 = (char *)b + (n1 * s);

	msort_with_tmp(b1, n1, s, cmp, t);
	msort_with_tmp(b2, n2, s, cmp, t);

	tmp = t;

	while (n1 > 0 && n2 > 0) {
		if (cmp(b1, b2) <= 0) {
			memcpy(tmp, b1, s);
			tmp += s;
			b1 += s;
			--n1;
		} else {
			memcpy(tmp, b2, s);
			tmp += s;
			b2 += s;
			--n2;
		}
	}
	if (n1 > 0)
		memcpy(tmp, b1, n1 * s);
	memcpy(b, t, (n - n2) * s);
}