(r < 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "xmlTextWriterWriteBase64() failed: %d", r);
			return (ARCHIVE_FATAL);
		}