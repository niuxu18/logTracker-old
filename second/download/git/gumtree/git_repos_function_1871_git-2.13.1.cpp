static int match_multi_number(unsigned long num, char c, const char *date,
			      char *end, struct tm *tm, time_t now)
{
	struct tm now_tm;
	struct tm *refuse_future;
	long num2, num3;

	num2 = strtol(end+1, &end, 10);
	num3 = -1;
	if (*end == c && isdigit(end[1]))
		num3 = strtol(end+1, &end, 10);

	/* Time? Date? */
	switch (c) {
	case ':':
		if (num3 < 0)
			num3 = 0;
		if (num < 25 && num2 >= 0 && num2 < 60 && num3 >= 0 && num3 <= 60) {
			tm->tm_hour = num;
			tm->tm_min = num2;
			tm->tm_sec = num3;
			break;
		}
		return 0;

	case '-':
	case '/':
	case '.':
		if (!now)
			now = time(NULL);
		refuse_future = NULL;
		if (gmtime_r(&now, &now_tm))
			refuse_future = &now_tm;

		if (num > 70) {
			/* yyyy-mm-dd? */
			if (is_date(num, num2, num3, NULL, now, tm))
				break;
			/* yyyy-dd-mm? */
			if (is_date(num, num3, num2, NULL, now, tm))
				break;
		}
		/* Our eastern European friends say dd.mm.yy[yy]
		 * is the norm there, so giving precedence to
		 * mm/dd/yy[yy] form only when separator is not '.'
		 */
		if (c != '.' &&
		    is_date(num3, num, num2, refuse_future, now, tm))
			break;
		/* European dd.mm.yy[yy] or funny US dd/mm/yy[yy] */
		if (is_date(num3, num2, num, refuse_future, now, tm))
			break;
		/* Funny European mm.dd.yy */
		if (c == '.' &&
		    is_date(num3, num, num2, refuse_future, now, tm))
			break;
		return 0;
	}
	return end - date;
}