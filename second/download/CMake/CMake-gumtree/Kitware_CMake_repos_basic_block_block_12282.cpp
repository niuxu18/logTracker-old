(argc < 2) {
			archive_set_error(a, ARCHIVE_ERRNO_FILE_FORMAT,
			    "Not enough arguments");
			return ARCHIVE_WARN;
		}