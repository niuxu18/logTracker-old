(written < 0) {
			archive_set_error(&a->archive, errno,
			    "Can't write to temporary file");
			return (ARCHIVE_FATAL);
		}