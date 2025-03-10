static int
lha_read_file_header_2(struct archive_read *a, struct lha *lha)
{
	const unsigned char *p;
	size_t extdsize;
	int err, padding;
	uint16_t header_crc;

	if ((p = __archive_read_ahead(a, H2_FIXED_SIZE, NULL)) == NULL)
		return (truncated_error(a));

	lha->header_size =archive_le16dec(p + H2_HEADER_SIZE_OFFSET);
	lha->compsize = archive_le32dec(p + H2_COMP_SIZE_OFFSET);
	lha->origsize = archive_le32dec(p + H2_ORIG_SIZE_OFFSET);
	lha->mtime = archive_le32dec(p + H2_TIME_OFFSET);
	lha->crc = archive_le16dec(p + H2_CRC_OFFSET);
	lha->setflag |= CRC_IS_SET;

	if (lha->header_size < H2_FIXED_SIZE) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "Invalid LHa header size");
		return (ARCHIVE_FATAL);
	}

	header_crc = lha_crc16(0, p, H2_FIXED_SIZE);
	__archive_read_consume(a, H2_FIXED_SIZE);

	/* Read extended headers */
	err = lha_read_file_extended_header(a, lha, &header_crc, 2,
		  lha->header_size - H2_FIXED_SIZE, &extdsize);
	if (err < ARCHIVE_WARN)
		return (err);

	/* Calculate a padding size. The result will be normally 0 or 1. */
	padding = (int)lha->header_size - (int)(H2_FIXED_SIZE + extdsize);
	if (padding > 0) {
		if ((p = __archive_read_ahead(a, padding, NULL)) == NULL)
			return (truncated_error(a));
		header_crc = lha_crc16(header_crc, p, padding);
		__archive_read_consume(a, padding);
	}

	if (header_crc != lha->header_crc) {
		archive_set_error(&a->archive, ARCHIVE_ERRNO_FILE_FORMAT,
		    "LHa header CRC error");
		return (ARCHIVE_FATAL);
	}
	return (err);
}