		zip->end_of_entry = 1;

	/* Set up a more descriptive format name. */
	archive_string_sprintf(&zip->format_name, "ZIP %d.%d (%s)",
	    version / 10, version % 10,
	    compression_name(zip->entry->compression));
	a->archive.archive_format_name = zip->format_name.s;

	return (ret);
}

static int
check_authentication_code(struct archive_read *a, const void *_p)
{
	struct zip *zip = (struct zip *)(a->format->data);

	/* Check authentication code. */
	if (zip->hctx_valid) {
		const void *p;
		uint8_t hmac[20];
		size_t hmac_len = 20;
		int cmp;

		archive_hmac_sha1_final(&zip->hctx, hmac, &hmac_len);
		if (_p == NULL) {
			/* Read authentication code. */
			p = __archive_read_ahead(a, AUTH_CODE_SIZE, NULL);
			if (p == NULL) {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_FILE_FORMAT,
				    "Truncated ZIP file data");
				return (ARCHIVE_FATAL);
			}
		} else {
			p = _p;
		}
		cmp = memcmp(hmac, p, AUTH_CODE_SIZE);
		__archive_read_consume(a, AUTH_CODE_SIZE);
		if (cmp != 0) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "ZIP bad Authentication code");
			return (ARCHIVE_WARN);
		}
	}
	return (ARCHIVE_OK);
}

/*
 * Read "uncompressed" data.  There are three cases:
 *  1) We know the size of the data.  This is always true for the
