{
			archive_set_error(&a->archive, ENOMEM,
			    "Can't allocate memory for Resource Fork");
			return (ARCHIVE_FATAL);
		}