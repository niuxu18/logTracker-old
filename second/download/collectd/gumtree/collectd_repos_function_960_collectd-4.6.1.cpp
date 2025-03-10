static int parse_part_number (void **ret_buffer, int *ret_buffer_len,
		uint64_t *value)
{
	char *buffer = *ret_buffer;
	int buffer_len = *ret_buffer_len;

	uint16_t tmp16;
	uint64_t tmp64;
	size_t exp_size = 2 * sizeof (uint16_t) + sizeof (uint64_t);

	uint16_t pkg_length;
	uint16_t pkg_type;

	if ((buffer_len < 0) || ((size_t) buffer_len < exp_size))
	{
		WARNING ("network plugin: parse_part_number: "
				"Packet too short: "
				"Chunk of size %u expected, "
				"but buffer has only %i bytes left.",
				(unsigned int) exp_size, buffer_len);
		return (-1);
	}

	memcpy ((void *) &tmp16, buffer, sizeof (tmp16));
	buffer += sizeof (tmp16);
	pkg_type = ntohs (tmp16);

	memcpy ((void *) &tmp16, buffer, sizeof (tmp16));
	buffer += sizeof (tmp16);
	pkg_length = ntohs (tmp16);

	memcpy ((void *) &tmp64, buffer, sizeof (tmp64));
	buffer += sizeof (tmp64);
	*value = ntohll (tmp64);

	*ret_buffer = buffer;
	*ret_buffer_len = buffer_len - pkg_length;

	return (0);
}