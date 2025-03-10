static int tree_write_stack_finish_subtree(struct tree_write_stack *tws)
{
	int ret;
	struct tree_write_stack *n = tws->next;
	unsigned char s[20];
	if (n) {
		ret = tree_write_stack_finish_subtree(n);
		if (ret)
			return ret;
		ret = write_sha1_file(n->buf.buf, n->buf.len, tree_type, s);
		if (ret)
			return ret;
		strbuf_release(&n->buf);
		free(n);
		tws->next = NULL;
		write_tree_entry(&tws->buf, 040000, tws->path, 2, s);
		tws->path[0] = tws->path[1] = '\0';
	}
	return 0;
}