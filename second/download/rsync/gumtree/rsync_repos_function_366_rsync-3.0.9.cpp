char *human_dnum(double dnum, int decimal_digits)
{
	char *buf = human_num(dnum);
	int len = strlen(buf);
	if (isDigit(buf + len - 1)) {
		/* There's extra room in buf prior to the start of the num. */
		buf -= decimal_digits + 2;
		snprintf(buf, len + decimal_digits + 3, "%.*f", decimal_digits, dnum);
	}
	return buf;
}