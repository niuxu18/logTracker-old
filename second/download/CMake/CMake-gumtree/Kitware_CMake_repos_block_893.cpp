{
		archive_set_error(&a->archive, errno,
		    "Could not allocate memory for extended attributes");
		ret = ARCHIVE_WARN;
		goto cleanup;
	}