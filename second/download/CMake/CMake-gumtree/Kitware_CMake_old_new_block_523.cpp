{
	(void)self; /* UNUSED */
	(void)cmd; /* UNUSED */

	archive_set_error(&self->archive->archive, -1,
	    "External compression programs not supported on this platform");
	return (ARCHIVE_FATAL);
}