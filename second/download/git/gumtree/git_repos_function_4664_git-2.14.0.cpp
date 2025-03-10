static const char *parse_long_magic(unsigned *magic, int *prefix_len,
				    struct pathspec_item *item,
				    const char *elem)
{
	const char *pos;
	const char *nextat;

	for (pos = elem + 2; *pos && *pos != ')'; pos = nextat) {
		size_t len = strcspn_escaped(pos, ",)");
		int i;

		if (pos[len] == ',')
			nextat = pos + len + 1; /* handle ',' */
		else
			nextat = pos + len; /* handle ')' and '\0' */

		if (!len)
			continue;

		if (starts_with(pos, "prefix:")) {
			char *endptr;
			*prefix_len = strtol(pos + 7, &endptr, 10);
			if (endptr - pos != len)
				die(_("invalid parameter for pathspec magic 'prefix'"));
			continue;
		}

		if (starts_with(pos, "attr:")) {
			char *attr_body = xmemdupz(pos + 5, len - 5);
			parse_pathspec_attr_match(item, attr_body);
			*magic |= PATHSPEC_ATTR;
			free(attr_body);
			continue;
		}

		for (i = 0; i < ARRAY_SIZE(pathspec_magic); i++) {
			if (strlen(pathspec_magic[i].name) == len &&
			    !strncmp(pathspec_magic[i].name, pos, len)) {
				*magic |= pathspec_magic[i].bit;
				break;
			}
		}

		if (ARRAY_SIZE(pathspec_magic) <= i)
			die(_("Invalid pathspec magic '%.*s' in '%s'"),
			    (int) len, pos, elem);
	}

	if (*pos != ')')
		die(_("Missing ')' at the end of pathspec magic in '%s'"),
		    elem);
	pos++;

	return pos;
}