{
				struct archive_wstring ws;

				archive_string_init(&ws);
				path_length = wcslen(wp);
				if (archive_wstring_ensure(&ws,
				    path_length + 2) == NULL) {
					archive_set_error(&a->archive, ENOMEM,
					    "Can't allocate pax data");
					archive_wstring_free(&ws);
					return(ARCHIVE_FATAL);
				}
				/* Should we keep '\' ? */
				if (wp[path_length -1] == L'\\')
					path_length--;
				archive_wstrncpy(&ws, wp, path_length);
				archive_wstrappend_wchar(&ws, L'/');
				archive_entry_copy_pathname_w(
				    entry_original, ws.s);
				archive_wstring_free(&ws);
				p = NULL;
			}