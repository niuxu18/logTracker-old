{
		/* src points to first char after '/' */
		if (src[0] == '\0') {
			break;
		} else if (src[0] == '/') {
			/* Found '//', ignore second one. */
			src++;
			continue;
		} else if (src[0] == '.') {
			if (src[1] == '\0') {
				/* Ignore trailing '.' */
				break;
			} else if (src[1] == '/') {
				/* Skip './'. */
				src += 2;
				continue;
			} else if (src[1] == '.') {
				if (src[2] == '/' || src[2] == '\0') {
					/* Conditionally warn about '..' */
					if (flags
					    & ARCHIVE_EXTRACT_SECURE_NODOTDOT) {
						fsobj_error(a_eno, a_estr,
						    ARCHIVE_ERRNO_MISC,
						    "Path contains ", "'..'");
						return (ARCHIVE_FAILED);
					}
				}
				/*
				 * Note: Under no circumstances do we
				 * remove '..' elements.  In
				 * particular, restoring
				 * '/foo/../bar/' should create the
				 * 'foo' dir as a side-effect.
				 */
			}
		}

		/* Copy current element, including leading '/'. */
		if (separator)
			*dest++ = '/';
		while (*src != '\0' && *src != '/') {
			*dest++ = *src++;
		}

		if (*src == '\0')
			break;

		/* Skip '/' separator. */
		separator = *src++;
	}