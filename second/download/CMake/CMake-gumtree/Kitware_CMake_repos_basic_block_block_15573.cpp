{
		free(strm);
		archive_set_error(a, ENOMEM,
		    "Can't allocate memory for PPMd");
		return (ARCHIVE_FATAL);
	}