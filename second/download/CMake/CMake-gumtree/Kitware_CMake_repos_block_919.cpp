{
		archive_set_error(&a->archive, errno,
		    "Couldn't query extended attribute");
		return (ARCHIVE_WARN);
	}