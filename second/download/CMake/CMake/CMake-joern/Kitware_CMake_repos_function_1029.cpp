int
archive_read_open_memory2(struct archive *a, const void *buff,
    size_t size, size_t read_size)
{
	struct read_memory_data *mine;

	mine = (struct read_memory_data *)calloc(1, sizeof(*mine));
	if (mine == NULL) {
		archive_set_error(a, ENOMEM, "No memory");
		return (ARCHIVE_FATAL);
	}
	mine->start = mine->p = (const unsigned char *)buff;
	mine->end = mine->start + size;
	mine->read_size = read_size;
	archive_read_set_open_callback(a, memory_read_open);
	archive_read_set_read_callback(a, memory_read);
	archive_read_set_seek_callback(a, memory_read_seek);
	archive_read_set_skip_callback(a, memory_read_skip);
	archive_read_set_close_callback(a, memory_read_close);
	archive_read_set_callback_data(a, mine);
	return (archive_read_open1(a));
}