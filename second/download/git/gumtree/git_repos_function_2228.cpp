int unquote_c_style(struct strbuf *sb, const char *quoted, const char **endp)
{
	size_t oldlen = sb->len, len;
	int ch, ac;

	if (*quoted++ != '"')
		return -1;

	for (;;) {
		len = strcspn(quoted, "\"\\");
		strbuf_add(sb, quoted, len);
		quoted += len;

		switch (*quoted++) {
		  case '"':
			if (endp)
				*endp = quoted;
			return 0;
		  case '\\':
			break;
		  default:
			goto error;
		}

		switch ((ch = *quoted++)) {
		case 'a': ch = '\a'; break;
		case 'b': ch = '\b'; break;
		case 'f': ch = '\f'; break;
		case 'n': ch = '\n'; break;
		case 'r': ch = '\r'; break;
		case 't': ch = '\t'; break;
		case 'v': ch = '\v'; break;

		case '\\': case '"':
			break; /* verbatim */

		/* octal values with first digit over 4 overflow */
		case '0': case '1': case '2': case '3':
					ac = ((ch - '0') << 6);
			if ((ch = *quoted++) < '0' || '7' < ch)
				goto error;
					ac |= ((ch - '0') << 3);
			if ((ch = *quoted++) < '0' || '7' < ch)
				goto error;
					ac |= (ch - '0');
					ch = ac;
					break;
				default:
			goto error;
			}
		strbuf_addch(sb, ch);
		}

  error:
	strbuf_setlen(sb, oldlen);
	return -1;
}