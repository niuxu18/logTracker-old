{
					archive_set_error(&a->archive,
					    ARCHIVE_ERRNO_MISC,
					    "Failed to encrypt file");
					return (ARCHIVE_FAILED);
				}