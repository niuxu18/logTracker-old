		die("Not a mark: %s", p);

	oe = find_mark(parse_mark_ref_eol(p));
	if (!oe)
		die("Unknown mark: %s", command_buf.buf);

	xsnprintf(output, sizeof(output), "%s\n", sha1_to_hex(oe->idx.sha1));
	cat_blob_write(output, 41);
}

static void parse_cat_blob(const char *p)
{
	struct object_entry *oe = oe;
