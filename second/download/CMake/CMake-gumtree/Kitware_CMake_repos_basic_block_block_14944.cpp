{
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate memory for a fixup");
		return (NULL);
	}