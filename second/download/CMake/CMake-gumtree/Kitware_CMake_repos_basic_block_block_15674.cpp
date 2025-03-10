{
	struct archive_write *a = (struct archive_write *)_a;
	struct cpio *cpio;

	archive_check_magic(_a, ARCHIVE_WRITE_MAGIC,
	    ARCHIVE_STATE_NEW, "archive_write_set_format_cpio_newc");

	/* If someone else was already registered, unregister them. */
	if (a->format_free != NULL)
		(a->format_free)(a);

	cpio = (struct cpio *)calloc(1, sizeof(*cpio));
	if (cpio == NULL) {
		archive_set_error(&a->archive, ENOMEM, "Can't allocate cpio data");
		return (ARCHIVE_FATAL);
	}
	a->format_data = cpio;
	a->format_name = "cpio";
	a->format_options = archive_write_newc_options;
	a->format_write_header = archive_write_newc_header;
	a->format_write_data = archive_write_newc_data;
	a->format_finish_entry = archive_write_newc_finish_entry;
	a->format_close = archive_write_newc_close;
	a->format_free = archive_write_newc_free;
	a->archive.archive_format = ARCHIVE_FORMAT_CPIO_SVR4_NOCRC;
	a->archive.archive_format_name = "SVR4 cpio nocrc";
	return (ARCHIVE_OK);
}