(s-- > 0) {
		*--p = (char)('0' + (v & 7));
		v >>= 3;
	}