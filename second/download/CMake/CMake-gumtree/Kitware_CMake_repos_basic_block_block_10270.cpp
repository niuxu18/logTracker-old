{
		archive_set_error(&(self->archive->archive),
		    ARCHIVE_ERRNO_MISC, "lz4 decompression failed");
		return (ARCHIVE_FATAL);
	}