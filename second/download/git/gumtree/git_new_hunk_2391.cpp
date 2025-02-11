	}
	item->date = parse_commit_date(bufptr, tail);

	return 0;
}

int parse_commit_gently(struct commit *item, int quiet_on_missing)
{
	enum object_type type;
	void *buffer;
	unsigned long size;
	int ret;

	if (!item)
		return -1;
	if (item->object.parsed)
		return 0;
	buffer = read_sha1_file(item->object.sha1, &type, &size);
	if (!buffer)
		return quiet_on_missing ? -1 :
			error("Could not read %s",
			     sha1_to_hex(item->object.sha1));
	if (type != OBJ_COMMIT) {
		free(buffer);
		return error("Object %s not a commit",
			     sha1_to_hex(item->object.sha1));
	}
