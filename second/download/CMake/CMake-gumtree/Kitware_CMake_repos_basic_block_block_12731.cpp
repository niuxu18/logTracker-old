{
	ssize_t bytes;
	int err;
	const char *h;
	const struct archive_entry_header_ustar *header;
	const struct archive_entry_header_gnutar *gnuheader;

	/* Loop until we find a workable header record. */
	for (;;) {
		tar_flush_unconsumed(a, unconsumed);

		/* Read 512-byte header record */
		h = __archive_read_ahead(a, 512, &bytes);
		if (bytes < 0)
			return ((int)bytes);
		if (bytes == 0) { /* EOF at a block boundary. */
			/* Some writers do omit the block of nulls. <sigh> */
			return (ARCHIVE_EOF);
		}
		if (bytes < 512) {  /* Short block at EOF; this is bad. */
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Truncated tar archive");
			return (ARCHIVE_FATAL);
		}
		*unconsumed = 512;

		/* Header is workable if it's not an end-of-archive mark. */
		if (h[0] != 0 || !archive_block_is_null(h))
			break;

		/* Ensure format is set for archives with only null blocks. */
		if (a->archive.archive_format_name == NULL) {
			a->archive.archive_format = ARCHIVE_FORMAT_TAR;
			a->archive.archive_format_name = "tar";
		}

		if (!tar->read_concatenated_archives) {
			/* Try to consume a second all-null record, as well. */
			tar_flush_unconsumed(a, unconsumed);
			h = __archive_read_ahead(a, 512, NULL);
			if (h != NULL && h[0] == 0 && archive_block_is_null(h))
				__archive_read_consume(a, 512);
			archive_clear_error(&a->archive);
			return (ARCHIVE_EOF);
		}

		/*
		 * We're reading concatenated archives, ignore this block and
		 * loop to get the next.
		 */
	}

	/*
	 * Note: If the checksum fails and we return ARCHIVE_RETRY,
	 * then the client is likely to just retry.  This is a very
	 * crude way to search for the next valid header!
	 *
	 * TODO: Improve this by implementing a real header scan.
	 */
	if (!checksum(a, h)) {
		tar_flush_unconsumed(a, unconsumed);
		archive_set_error(&a->archive, EINVAL, "Damaged tar archive");
		return (ARCHIVE_RETRY); /* Retryable: Invalid header */
	}

	if (++tar->header_recursion_depth > 32) {
		tar_flush_unconsumed(a, unconsumed);
		archive_set_error(&a->archive, EINVAL, "Too many special headers");
		return (ARCHIVE_WARN);
	}

	/* Determine the format variant. */
	header = (const struct archive_entry_header_ustar *)h;

	switch(header->typeflag[0]) {
	case 'A': /* Solaris tar ACL */
		a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE;
		a->archive.archive_format_name = "Solaris tar";
		err = header_Solaris_ACL(a, tar, entry, h, unconsumed);
		break;
	case 'g': /* POSIX-standard 'g' header. */
		a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE;
		a->archive.archive_format_name = "POSIX pax interchange format";
		err = header_pax_global(a, tar, entry, h, unconsumed);
		if (err == ARCHIVE_EOF)
			return (err);
		break;
	case 'K': /* Long link name (GNU tar, others) */
		err = header_longlink(a, tar, entry, h, unconsumed);
		break;
	case 'L': /* Long filename (GNU tar, others) */
		err = header_longname(a, tar, entry, h, unconsumed);
		break;
	case 'V': /* GNU volume header */
		err = header_volume(a, tar, entry, h, unconsumed);
		break;
	case 'X': /* Used by SUN tar; same as 'x'. */
		a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE;
		a->archive.archive_format_name =
		    "POSIX pax interchange format (Sun variant)";
		err = header_pax_extensions(a, tar, entry, h, unconsumed);
		break;
	case 'x': /* POSIX-standard 'x' header. */
		a->archive.archive_format = ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE;
		a->archive.archive_format_name = "POSIX pax interchange format";
		err = header_pax_extensions(a, tar, entry, h, unconsumed);
		break;
	default:
		gnuheader = (const struct archive_entry_header_gnutar *)h;
		if (memcmp(gnuheader->magic, "ustar  \0", 8) == 0) {
			a->archive.archive_format = ARCHIVE_FORMAT_TAR_GNUTAR;
			a->archive.archive_format_name = "GNU tar format";
			err = header_gnutar(a, tar, entry, h, unconsumed);
		} else if (memcmp(header->magic, "ustar", 5) == 0) {
			if (a->archive.archive_format != ARCHIVE_FORMAT_TAR_PAX_INTERCHANGE) {
				a->archive.archive_format = ARCHIVE_FORMAT_TAR_USTAR;
				a->archive.archive_format_name = "POSIX ustar format";
			}
			err = header_ustar(a, tar, entry, h);
		} else {
			a->archive.archive_format = ARCHIVE_FORMAT_TAR;
			a->archive.archive_format_name = "tar (non-POSIX)";
			err = header_old_tar(a, tar, entry, h);
		}
	}
	if (err == ARCHIVE_FATAL)
		return (err);

	tar_flush_unconsumed(a, unconsumed);

	h = NULL;
	header = NULL;

	--tar->header_recursion_depth;
	/* Yuck.  Apple's design here ends up storing long pathname
	 * extensions for both the AppleDouble extension entry and the
	 * regular entry.
	 */
	if ((err == ARCHIVE_WARN || err == ARCHIVE_OK) &&
	    tar->header_recursion_depth == 0 &&
	    tar->process_mac_extensions) {
		int err2 = read_mac_metadata_blob(a, tar, entry, h, unconsumed);
		if (err2 < err)
			err = err2;
	}

	/* We return warnings or success as-is.  Anything else is fatal. */
	if (err == ARCHIVE_WARN || err == ARCHIVE_OK) {
		if (tar->sparse_gnu_pending) {
			if (tar->sparse_gnu_major == 1 &&
			    tar->sparse_gnu_minor == 0) {
				ssize_t bytes_read;

				tar->sparse_gnu_pending = 0;
				/* Read initial sparse map. */
				bytes_read = gnu_sparse_10_read(a, tar, unconsumed);
				if (bytes_read < 0)
					return ((int)bytes_read);
				tar->entry_bytes_remaining -= bytes_read;
			} else {
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "Unrecognized GNU sparse file format");
				return (ARCHIVE_WARN);
			}
			tar->sparse_gnu_pending = 0;
		}
		return (err);
	}
	if (err == ARCHIVE_EOF)
		/* EOF when recursively reading a header is bad. */
		archive_set_error(&a->archive, EINVAL, "Damaged tar archive");
	return (ARCHIVE_FATAL);
}