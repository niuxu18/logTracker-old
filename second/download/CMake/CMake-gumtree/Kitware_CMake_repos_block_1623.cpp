{
				if (r == ARCHIVE_FATAL)
					return (r);
				err = r;
				archive_set_error(&a->archive,
				    ARCHIVE_ERRNO_MISC,
				    "Parse error: SUN.holesdata");
			}