{
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "Failed to clean up bzip2 decompressor");
			r = ARCHIVE_FATAL;
		}