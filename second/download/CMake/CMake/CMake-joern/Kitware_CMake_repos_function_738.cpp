static ssize_t
zisofs_extract_init(struct archive_write *a, struct zisofs_extract *zisofs,
    const unsigned char *p, size_t bytes)
{
	size_t avail = bytes;
	size_t _ceil, xsize;

	/* Allocate block pointers buffer. */
	_ceil = (size_t)((zisofs->pz_uncompressed_size +
		(((int64_t)1) << zisofs->pz_log2_bs) - 1)
		>> zisofs->pz_log2_bs);
	xsize = (_ceil + 1) * 4;
	if (zisofs->block_pointers == NULL) {
		size_t alloc = ((xsize >> 10) + 1) << 10;
		zisofs->block_pointers = malloc(alloc);
		if (zisofs->block_pointers == NULL) {
			archive_set_error(&a->archive, ENOMEM,
			    "No memory for zisofs decompression");
			return (ARCHIVE_FATAL);
		}
	}
	zisofs->block_pointers_size = xsize;

	/* Allocate uncompressed data buffer. */
	zisofs->uncompressed_buffer_size = (size_t)1UL << zisofs->pz_log2_bs;

	/*
	 * Read the file header, and check the magic code of zisofs.
	 */
	if (!zisofs->header_passed) {
		int err = 0;
		if (avail < 16) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Illegal zisofs file body");
			return (ARCHIVE_FATAL);
		}

		if (memcmp(p, zisofs_magic, sizeof(zisofs_magic)) != 0)
			err = 1;
		else if (archive_le32dec(p + 8) != zisofs->pz_uncompressed_size)
			err = 1;
		else if (p[12] != 4 || p[13] != zisofs->pz_log2_bs)
			err = 1;
		if (err) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_FILE_FORMAT,
			    "Illegal zisofs file body");
			return (ARCHIVE_FATAL);
		}
		avail -= 16;
		p += 16;
		zisofs->header_passed = 1;
	}

	/*
	 * Read block pointers.
	 */
	if (zisofs->header_passed &&
	    zisofs->block_pointers_avail < zisofs->block_pointers_size) {
		xsize = zisofs->block_pointers_size
		    - zisofs->block_pointers_avail;
		if (avail < xsize)
			xsize = avail;
		memcpy(zisofs->block_pointers
		    + zisofs->block_pointers_avail, p, xsize);
		zisofs->block_pointers_avail += xsize;
		avail -= xsize;
	    	if (zisofs->block_pointers_avail
		    == zisofs->block_pointers_size) {
			/* We've got all block pointers and initialize
			 * related variables.	*/
			zisofs->block_off = 0;
			zisofs->block_avail = 0;
			/* Complete a initialization */
			zisofs->initialized = 1;
		}
	}
	return ((ssize_t)avail);
}