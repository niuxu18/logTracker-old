int move_window(struct sliding_view *view, off_t off, size_t width)
{
	off_t file_offset;
	assert(view);
	assert(view->width <= view->buf.len);
	assert(!check_offset_overflow(view->off, view->buf.len));

	if (check_offset_overflow(off, width))
		return -1;
	if (off < view->off || off + width < view->off + view->width)
		return error("invalid delta: window slides left");
	if (view->max_off >= 0 && view->max_off < off + (off_t) width)
		return error("delta preimage ends early");

	file_offset = view->off + view->buf.len;
	if (off < file_offset) {
		/* Move the overlapping region into place. */
		strbuf_remove(&view->buf, 0, off - view->off);
	} else {
		/* Seek ahead to skip the gap. */
		if (skip_or_whine(view->file, off - file_offset))
			return -1;
		strbuf_setlen(&view->buf, 0);
	}

	if (view->buf.len > width)
		; /* Already read. */
	else if (read_to_fill_or_whine(view->file, &view->buf, width))
		return -1;

	view->off = off;
	view->width = width;
	return 0;
}