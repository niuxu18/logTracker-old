{
	struct ar *ar;
	char *p;
	size_t size;

	ar = (struct ar*)(a->format->data);
	size = ar->strtab_size;

	for (p = ar->strtab; p < ar->strtab + size - 1; ++p) {
		if (*p == '/') {
			*p++ = '\0';
			if (*p != '\n')
				goto bad_string_table;
			*p = '\0';
		}
	}
	/*
	 * GNU ar always pads the table to an even size.
	 * The pad character is either '\n' or '`'.
	 */
	if (p != ar->strtab + size && *p != '\n' && *p != '`')
		goto bad_string_table;

	/* Enforce zero termination. */
	ar->strtab[size - 1] = '\0';

	return (ARCHIVE_OK);

bad_string_table:
	archive_set_error(&a->archive, EINVAL,
	    "Invalid string table");
	free(ar->strtab);
	ar->strtab = NULL;
	return (ARCHIVE_FATAL);
}