static void handle_node(void)
{
	const uint32_t type = node_ctx.type;
	const int have_props = node_ctx.prop_length != -1;
	const int have_text = node_ctx.text_length != -1;
	/*
	 * Old text for this node:
	 *  NULL	- directory or bug
	 *  empty_blob	- empty
	 *  "<dataref>"	- data retrievable from fast-import
	 */
	static const char *const empty_blob = "::empty::";
	const char *old_data = NULL;
	uint32_t old_mode = S_IFREG | 0644;

	if (node_ctx.action == NODEACT_DELETE) {
		if (have_text || have_props || node_ctx.srcRev)
			die("invalid dump: deletion node has "
				"copyfrom info, text, or properties");
		fast_export_delete(node_ctx.dst.buf);
		return;
	}
	if (node_ctx.action == NODEACT_REPLACE) {
		fast_export_delete(node_ctx.dst.buf);
		node_ctx.action = NODEACT_ADD;
	}
	if (node_ctx.srcRev) {
		fast_export_copy(node_ctx.srcRev, node_ctx.src.buf, node_ctx.dst.buf);
		if (node_ctx.action == NODEACT_ADD)
			node_ctx.action = NODEACT_CHANGE;
	}
	if (have_text && type == S_IFDIR)
		die("invalid dump: directories cannot have text attached");

	/*
	 * Find old content (old_data) and decide on the new mode.
	 */
	if (node_ctx.action == NODEACT_CHANGE && !*node_ctx.dst.buf) {
		if (type != S_IFDIR)
			die("invalid dump: root of tree is not a regular file");
		old_data = NULL;
	} else if (node_ctx.action == NODEACT_CHANGE) {
		uint32_t mode;
		old_data = fast_export_read_path(node_ctx.dst.buf, &mode);
		if (mode == S_IFDIR && type != S_IFDIR)
			die("invalid dump: cannot modify a directory into a file");
		if (mode != S_IFDIR && type == S_IFDIR)
			die("invalid dump: cannot modify a file into a directory");
		node_ctx.type = mode;
		old_mode = mode;
	} else if (node_ctx.action == NODEACT_ADD) {
		if (type == S_IFDIR)
			old_data = NULL;
		else if (have_text)
			old_data = empty_blob;
		else
			die("invalid dump: adds node without text");
	} else {
		die("invalid dump: Node-path block lacks Node-action");
	}

	/*
	 * Adjust mode to reflect properties.
	 */
	if (have_props) {
		if (!node_ctx.prop_delta)
			node_ctx.type = type;
		if (node_ctx.prop_length)
			read_props();
	}

	/*
	 * Save the result.
	 */
	if (type == S_IFDIR)	/* directories are not tracked. */
		return;
	assert(old_data);
	if (old_data == empty_blob)
		/* For the fast_export_* functions, NULL means empty. */
		old_data = NULL;
	if (!have_text) {
		fast_export_modify(node_ctx.dst.buf, node_ctx.type, old_data);
		return;
	}
	if (!node_ctx.text_delta) {
		fast_export_modify(node_ctx.dst.buf, node_ctx.type, "inline");
		fast_export_data(node_ctx.type, node_ctx.text_length, &input);
		return;
	}
	fast_export_modify(node_ctx.dst.buf, node_ctx.type, "inline");
	fast_export_blob_delta(node_ctx.type, old_mode, old_data,
				node_ctx.text_length, &input);
}