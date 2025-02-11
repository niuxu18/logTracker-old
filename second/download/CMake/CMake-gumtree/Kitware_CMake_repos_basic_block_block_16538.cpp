{
	struct iso9660 *iso9660 = a->format_data;
	struct isofile *file = iso9660->cur_file;
	const unsigned char *p, *endp;
	const unsigned char *magic_buff;
	uint32_t uncompressed_size;
	unsigned char header_size;
	unsigned char log2_bs;
	size_t _ceil, doff;
	uint32_t bst, bed;
	int magic_max;
	int64_t entry_size;

	entry_size = archive_entry_size(file->entry);
	if ((int64_t)sizeof(iso9660->zisofs.magic_buffer) > entry_size)
		magic_max = (int)entry_size;
	else
		magic_max = sizeof(iso9660->zisofs.magic_buffer);

	if (iso9660->zisofs.magic_cnt == 0 && s >= (size_t)magic_max)
		/* It's unnecessary we copy buffer. */
		magic_buff = buff;
	else {
		if (iso9660->zisofs.magic_cnt < magic_max) {
			size_t l;

			l = sizeof(iso9660->zisofs.magic_buffer)
			    - iso9660->zisofs.magic_cnt;
			if (l > s)
				l = s;
			memcpy(iso9660->zisofs.magic_buffer
			    + iso9660->zisofs.magic_cnt, buff, l);
			iso9660->zisofs.magic_cnt += (int)l;
			if (iso9660->zisofs.magic_cnt < magic_max)
				return;
		}
		magic_buff = iso9660->zisofs.magic_buffer;
	}
	iso9660->zisofs.detect_magic = 0;
	p = magic_buff;

	/* Check the magic code of zisofs. */
	if (memcmp(p, zisofs_magic, sizeof(zisofs_magic)) != 0)
		/* This is not zisofs file which made by mkzftree. */
		return;
	p += sizeof(zisofs_magic);

	/* Read a zisofs header. */
	uncompressed_size = archive_le32dec(p);
	header_size = p[4];
	log2_bs = p[5];
	if (uncompressed_size < 24 || header_size != 4 ||
	    log2_bs > 30 || log2_bs < 7)
		return;/* Invalid or not supported header. */

	/* Calculate a size of Block Pointers of zisofs. */
	_ceil = (uncompressed_size +
	        (ARCHIVE_LITERAL_LL(1) << log2_bs) -1) >> log2_bs;
	doff = (_ceil + 1) * 4 + 16;
	if (entry_size < (int64_t)doff)
		return;/* Invalid data. */

	/* Check every Block Pointer has valid value. */
	p = magic_buff + 16;
	endp = magic_buff + magic_max;
	while (_ceil && p + 8 <= endp) {
		bst = archive_le32dec(p);
		if (bst != doff)
			return;/* Invalid data. */
		p += 4;
		bed = archive_le32dec(p);
		if (bed < bst || bed > entry_size)
			return;/* Invalid data. */
		doff += bed - bst;
		_ceil--;
	}

	file->zisofs.uncompressed_size = uncompressed_size;
	file->zisofs.header_size = header_size;
	file->zisofs.log2_bs = log2_bs;

	/* Disable making a zisofs image. */
	iso9660->zisofs.making = 0;
}