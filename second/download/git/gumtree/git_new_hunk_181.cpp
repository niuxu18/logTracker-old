		strbuf_attach(&last_blob.data, buf, size, size);
		last_blob.depth = oe->depth;
	} else
		free(buf);
}

static void parse_cat_blob(const char *p)
{
	struct object_entry *oe = oe;
	unsigned char sha1[20];

	/* cat-blob SP <object> LF */
	if (*p == ':') {
		oe = find_mark(parse_mark_ref_eol(p));
		if (!oe)
			die("Unknown mark: %s", command_buf.buf);
		hashcpy(sha1, oe->idx.sha1);
	} else {
