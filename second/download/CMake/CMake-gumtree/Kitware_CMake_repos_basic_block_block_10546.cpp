(zip->entry_bytes_remaining > 0) {
			const void *buff;
			unsigned char *mem;
			size_t size;
			int64_t offset;

			r = archive_read_format_7zip_read_data(a, &buff,
				&size, &offset);
			if (r < ARCHIVE_WARN) {
				free(symname);
				return (r);
			}
			mem = realloc(symname, symsize + size + 1);
			if (mem == NULL) {
				free(symname);
				archive_set_error(&a->archive, ENOMEM,
				    "Can't allocate memory for Symname");
				return (ARCHIVE_FATAL);
			}
			symname = mem;
			memcpy(symname+symsize, buff, size);
			symsize += size;
		}