{
		archive_set_error(a, ENOMEM,
		    "Can't allocate memory for xz stream");
		return (ARCHIVE_FATAL);
	}