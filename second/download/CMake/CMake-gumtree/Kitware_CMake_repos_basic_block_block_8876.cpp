{
	struct archive *ar;
	struct archive_entry *ae;
	struct archive_string as;
	const void *buff;
	size_t size;
	int64_t offset;
	int r;

	ar = archive_read_new(); 
	if (ar == NULL) {
		archive_set_error(&(a->archive), ENOMEM, "No memory");
		return (ARCHIVE_FATAL);
	}
	r = archive_read_support_format_raw(ar);
	r = archive_read_support_format_empty(ar);
	if (r != ARCHIVE_OK) {
		archive_copy_error(&(a->archive), ar);
		archive_read_free(ar);
		return (r);
	}
	if (mbs)
		r = archive_read_open_filename(ar, pathname, 512*20);
	else
		r = archive_read_open_filename_w(ar, pathname, 512*20);
	if (r != ARCHIVE_OK) {
		archive_copy_error(&(a->archive), ar);
		archive_read_free(ar);
		return (r);
	}
	r = archive_read_next_header(ar, &ae);
	if (r != ARCHIVE_OK) {
		archive_read_free(ar);
		if (r == ARCHIVE_EOF) {
			return (ARCHIVE_OK);
		} else {
			archive_copy_error(&(a->archive), ar);
			return (r);
		}
	}

	archive_string_init(&as);

	while ((r = archive_read_data_block(ar, &buff, &size, &offset))
	    == ARCHIVE_OK) {
		const char *b = (const char *)buff;

		while (size) {
			const char *s = (const char *)b;
			size_t length = 0;
			int found_separator = 0;

			while (length < size) {
				if (nullSeparator) {
					if (*b == '\0') {
						found_separator = 1;
						break;
					}
				} else {
			            	if (*b == 0x0d || *b == 0x0a) {
						found_separator = 1;
						break;
					}
				}
				b++;
				length++;
			}
			if (!found_separator) {
				archive_strncat(&as, s, length);
				/* Read next data block. */
				break;
			}
			b++;
			size -= length + 1;
			archive_strncat(&as, s, length);

			/* If the line is not empty, add the pattern. */
			if (archive_strlen(&as) > 0) {
				/* Add pattern. */
				r = add_pattern_mbs(a, mlist, as.s);
				if (r != ARCHIVE_OK) {
					archive_read_free(ar);
					archive_string_free(&as);
					return (r);
				}
				archive_string_empty(&as);
			}
		}
	}

	/* If an error occurred, report it immediately. */
	if (r < ARCHIVE_OK) {
		archive_copy_error(&(a->archive), ar);
		archive_read_free(ar);
		archive_string_free(&as);
		return (r);
	}

	/* If the line is not empty, add the pattern. */
	if (r == ARCHIVE_EOF && archive_strlen(&as) > 0) {
		/* Add pattern. */
		r = add_pattern_mbs(a, mlist, as.s);
		if (r != ARCHIVE_OK) {
			archive_read_free(ar);
			archive_string_free(&as);
			return (r);
		}
	}
	archive_read_free(ar);
	archive_string_free(&as);
	return (ARCHIVE_OK);
}