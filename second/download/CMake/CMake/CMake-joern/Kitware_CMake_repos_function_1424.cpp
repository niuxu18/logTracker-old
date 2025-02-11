static int
archive_format_gnutar_header(struct archive_write *a, char h[512],
    struct archive_entry *entry, int tartype)
{
	unsigned int checksum;
	int i, ret;
	size_t copy_length;
	const char *p;
	struct gnutar *gnutar;

	gnutar = (struct gnutar *)a->format_data;

	ret = 0;

	/*
	 * The "template header" already includes the signature,
	 * various end-of-field markers, and other required elements.
	 */
	memcpy(h, &template_header, 512);

	/*
	 * Because the block is already null-filled, and strings
	 * are allowed to exactly fill their destination (without null),
	 * I use memcpy(dest, src, strlen()) here a lot to copy strings.
	 */

	if (tartype == 'K' || tartype == 'L') {
		p = archive_entry_pathname(entry);
		copy_length = strlen(p);
	} else {
		p = gnutar->pathname;
		copy_length = gnutar->pathname_length;
	}
	if (copy_length > GNUTAR_name_size)
		copy_length = GNUTAR_name_size;
	memcpy(h + GNUTAR_name_offset, p, copy_length);

	if ((copy_length = gnutar->linkname_length) > 0) {
		if (copy_length > GNUTAR_linkname_size)
			copy_length = GNUTAR_linkname_size;
		memcpy(h + GNUTAR_linkname_offset, gnutar->linkname,
		    copy_length);
	}

	/* TODO: How does GNU tar handle unames longer than GNUTAR_uname_size? */
	if (tartype == 'K' || tartype == 'L') {
		p = archive_entry_uname(entry);
		copy_length = strlen(p);
	} else {
		p = gnutar->uname;
		copy_length = gnutar->uname_length;
	}
	if (copy_length > 0) {
		if (copy_length > GNUTAR_uname_size)
			copy_length = GNUTAR_uname_size;
		memcpy(h + GNUTAR_uname_offset, p, copy_length);
	}

	/* TODO: How does GNU tar handle gnames longer than GNUTAR_gname_size? */
	if (tartype == 'K' || tartype == 'L') {
		p = archive_entry_gname(entry);
		copy_length = strlen(p);
	} else {
		p = gnutar->gname;
		copy_length = gnutar->gname_length;
	}
	if (copy_length > 0) {
		if (strlen(p) > GNUTAR_gname_size)
			copy_length = GNUTAR_gname_size;
		memcpy(h + GNUTAR_gname_offset, p, copy_length);
	}

	/* By truncating the mode here, we ensure it always fits. */
	format_octal(archive_entry_mode(entry) & 07777,
	    h + GNUTAR_mode_offset, GNUTAR_mode_size);

	/* GNU tar supports base-256 here, so should never overflow. */
	if (format_number(archive_entry_uid(entry), h + GNUTAR_uid_offset,
		GNUTAR_uid_size, GNUTAR_uid_max_size)) {
		archive_set_error(&a->archive, ERANGE,
		    "Numeric user ID %jd too large",
		    (intmax_t)archive_entry_uid(entry));
		ret = ARCHIVE_FAILED;
	}

	/* GNU tar supports base-256 here, so should never overflow. */
	if (format_number(archive_entry_gid(entry), h + GNUTAR_gid_offset,
		GNUTAR_gid_size, GNUTAR_gid_max_size)) {
		archive_set_error(&a->archive, ERANGE,
		    "Numeric group ID %jd too large",
		    (intmax_t)archive_entry_gid(entry));
		ret = ARCHIVE_FAILED;
	}

	/* GNU tar supports base-256 here, so should never overflow. */
	if (format_number(archive_entry_size(entry), h + GNUTAR_size_offset,
		GNUTAR_size_size, GNUTAR_size_max_size)) {
		archive_set_error(&a->archive, ERANGE,
		    "File size out of range");
		ret = ARCHIVE_FAILED;
	}

	/* Shouldn't overflow before 2106, since mtime field is 33 bits. */
	format_octal(archive_entry_mtime(entry),
	    h + GNUTAR_mtime_offset, GNUTAR_mtime_size);

	if (archive_entry_filetype(entry) == AE_IFBLK
	    || archive_entry_filetype(entry) == AE_IFCHR) {
		if (format_octal(archive_entry_rdevmajor(entry),
		    h + GNUTAR_rdevmajor_offset,
			GNUTAR_rdevmajor_size)) {
			archive_set_error(&a->archive, ERANGE,
			    "Major device number too large");
			ret = ARCHIVE_FAILED;
		}

		if (format_octal(archive_entry_rdevminor(entry),
		    h + GNUTAR_rdevminor_offset,
			GNUTAR_rdevminor_size)) {
			archive_set_error(&a->archive, ERANGE,
			    "Minor device number too large");
			ret = ARCHIVE_FAILED;
		}
	}

	h[GNUTAR_typeflag_offset] = tartype;

	checksum = 0;
	for (i = 0; i < 512; i++)
		checksum += 255 & (unsigned int)h[i];
	h[GNUTAR_checksum_offset + 6] = '\0'; /* Can't be pre-set in the template. */
	/* h[GNUTAR_checksum_offset + 7] = ' '; */ /* This is pre-set in the template. */
	format_octal(checksum, h + GNUTAR_checksum_offset, 6);
	return (ret);
}