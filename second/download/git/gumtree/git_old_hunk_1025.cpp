
void bitmap_writer_finish(struct pack_idx_entry **index,
			  uint32_t index_nr,
			  const char *filename,
			  uint16_t options)
{
	static char tmp_file[PATH_MAX];
	static uint16_t default_version = 1;
	static uint16_t flags = BITMAP_OPT_FULL_DAG;
	struct sha1file *f;

	struct bitmap_disk_header header;

	int fd = odb_mkstemp(tmp_file, sizeof(tmp_file), "pack/tmp_bitmap_XXXXXX");

	if (fd < 0)
		die_errno("unable to create '%s'", tmp_file);
	f = sha1fd(fd, tmp_file);

	memcpy(header.magic, BITMAP_IDX_SIGNATURE, sizeof(BITMAP_IDX_SIGNATURE));
	header.version = htons(default_version);
	header.options = htons(flags | options);
	header.entry_count = htonl(writer.selected_nr);
	hashcpy(header.checksum, writer.pack_checksum);
