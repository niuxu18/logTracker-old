(char_cnt-- > 0) {
		if (*p >= 'a' && *p <= 'f')
			digit = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F')
			digit = *p - 'A' + 10;
		else if (*p >= '0' && *p <= '9')
			digit = *p - '0';
		else
			return (l);
		p++;
		l <<= 4;
		l |= digit;
	}