{
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Couldn't find out CAB header");
				return (ARCHIVE_FATAL);
			}