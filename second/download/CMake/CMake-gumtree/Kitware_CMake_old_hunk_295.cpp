		return (ARCHIVE_FATAL);
	}

	ustar = (struct ustar *)malloc(sizeof(*ustar));
	if (ustar == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate ustar data");
		return (ARCHIVE_FATAL);
	}
	memset(ustar, 0, sizeof(*ustar));
	a->format_data = ustar;
	a->format_name = "ustar";
	a->format_options = archive_write_ustar_options;
