{
		archive_set_error(f->archive, ENOMEM,
		    "Can't allocate data for uuencode buffer");
		return (ARCHIVE_FATAL);
	}