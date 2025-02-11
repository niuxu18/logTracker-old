void strbuf_utf8_replace(struct strbuf *sb_src, int pos, int width,
			 const char *subst)
{
	struct strbuf sb_dst = STRBUF_INIT;
	char *src = sb_src->buf;
	char *end = src + sb_src->len;
	char *dst;
	int w = 0, subst_len = 0;

	if (subst)
		subst_len = strlen(subst);
	strbuf_grow(&sb_dst, sb_src->len + subst_len);
	dst = sb_dst.buf;

	while (src < end) {
		char *old;
		size_t n;

		while ((n = display_mode_esc_sequence_len(src))) {
			memcpy(dst, src, n);
			src += n;
			dst += n;
		}

		old = src;
		n = utf8_width((const char**)&src, NULL);
		if (!src) 	/* broken utf-8, do nothing */
			return;
		if (n && w >= pos && w < pos + width) {
			if (subst) {
				memcpy(dst, subst, subst_len);
				dst += subst_len;
				subst = NULL;
			}
			w += n;
			continue;
		}
		memcpy(dst, old, src - old);
		dst += src - old;
		w += n;
	}
	strbuf_setlen(&sb_dst, dst - sb_dst.buf);
	strbuf_swap(sb_src, &sb_dst);
	strbuf_release(&sb_dst);
}