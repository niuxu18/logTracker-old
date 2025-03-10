{
	const unsigned char *p;
	ssize_t logical_block_size;
	int32_t volume_block;
	int32_t location;

	(void)iso9660; /* UNUSED */

	/* Type of the Enhanced Volume Descriptor must be 2. */
	if (h[PVD_type_offset] != 2)
		return (0);

	/* EVD version must be 2. */
	if (h[PVD_version_offset] != 2)
		return (0);

	/* Reserved field must be 0. */
	if (h[PVD_reserved1_offset] != 0)
		return (0);

	/* Reserved field must be 0. */
	if (!isNull(iso9660, h, PVD_reserved2_offset, PVD_reserved2_size))
		return (0);

	/* Reserved field must be 0. */
	if (!isNull(iso9660, h, PVD_reserved3_offset, PVD_reserved3_size))
		return (0);

	/* Logical block size must be > 0. */
	/* I've looked at Ecma 119 and can't find any stronger
	 * restriction on this field. */
	logical_block_size =
	    archive_le16dec(h + PVD_logical_block_size_offset);
	if (logical_block_size <= 0)
		return (0);

	volume_block =
	    archive_le32dec(h + PVD_volume_space_size_offset);
	if (volume_block <= SYSTEM_AREA_BLOCK+4)
		return (0);

	/* File structure version must be 2 for ISO9660:1999. */
	if (h[PVD_file_structure_version_offset] != 2)
		return (0);

	/* Location of Occurrence of Type L Path Table must be
	 * available location,
	 * >= SYSTEM_AREA_BLOCK(16) + 2 and < Volume Space Size. */
	location = archive_le32dec(h+PVD_type_1_path_table_offset);
	if (location < SYSTEM_AREA_BLOCK+2 || location >= volume_block)
		return (0);

	/* Location of Occurrence of Type M Path Table must be
	 * available location,
	 * >= SYSTEM_AREA_BLOCK(16) + 2 and < Volume Space Size. */
	location = archive_be32dec(h+PVD_type_m_path_table_offset);
	if ((location > 0 && location < SYSTEM_AREA_BLOCK+2)
	    || location >= volume_block)
		return (0);

	/* Reserved field must be 0. */
	if (!isNull(iso9660, h, PVD_reserved4_offset, PVD_reserved4_size))
		return (0);

	/* Reserved field must be 0. */
	if (!isNull(iso9660, h, PVD_reserved5_offset, PVD_reserved5_size))
		return (0);

	/* Read Root Directory Record in Volume Descriptor. */
	p = h + PVD_root_directory_record_offset;
	if (p[DR_length_offset] != 34)
		return (0);

	return (48);
}