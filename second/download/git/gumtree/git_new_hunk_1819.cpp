		die("Missing > in ident string: %s", buf);
	ltgt++;
	if (*ltgt != ' ')
		die("Missing space after > in ident string: %s", buf);
	ltgt++;
	name_len = ltgt - buf;
	strbuf_add(&ident, buf, name_len);

	switch (whenspec) {
	case WHENSPEC_RAW:
		if (validate_raw_date(ltgt, &ident) < 0)
			die("Invalid raw date \"%s\" in ident: %s", ltgt, buf);
		break;
	case WHENSPEC_RFC2822:
		if (parse_date(ltgt, &ident) < 0)
			die("Invalid rfc2822 date \"%s\" in ident: %s", ltgt, buf);
		break;
	case WHENSPEC_NOW:
		if (strcmp("now", ltgt))
			die("Date in ident must be 'now': %s", buf);
		datestamp(&ident);
		break;
	}

	return strbuf_detach(&ident, NULL);
}

static void parse_and_store_blob(
	struct last_object *last,
	unsigned char *sha1out,
	uintmax_t mark)
