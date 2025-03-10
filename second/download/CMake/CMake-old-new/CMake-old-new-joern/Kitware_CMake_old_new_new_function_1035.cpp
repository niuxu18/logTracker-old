static const unsigned char *
header_bytes(struct archive_read *a, size_t rbytes)
{
	struct _7zip *zip = (struct _7zip *)a->format->data;
	const unsigned char *p;

	if (zip->header_bytes_remaining < rbytes)
		return (NULL);
	if (zip->pack_stream_bytes_unconsumed)
		read_consume(a);

	if (zip->header_is_encoded == 0) {
		p = __archive_read_ahead(a, rbytes, NULL);
		if (p == NULL)
			return (NULL);
		zip->header_bytes_remaining -= rbytes;
		zip->pack_stream_bytes_unconsumed = rbytes;
	} else {
		const void *buff;
		ssize_t bytes;

		bytes = read_stream(a, &buff, rbytes, rbytes);
		if (bytes <= 0)
			return (NULL);
		zip->header_bytes_remaining -= bytes;
		p = buff;
	}

	/* Update checksum */
	zip->header_crc32 = crc32(zip->header_crc32, p, rbytes);
	return (p);
}