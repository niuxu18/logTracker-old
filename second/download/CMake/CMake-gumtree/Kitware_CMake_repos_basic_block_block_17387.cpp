(i = 0; i < n; i++) {
			r = xmlwrite_string(a, writer,
			    avail[i]->xarname, NULL);
			if (r != ARCHIVE_OK)
				return (r);
		}