static void parse_reset_branch(const char *arg)
{
	struct branch *b;

	b = lookup_branch(arg);
	if (b) {
		oidclr(&b->oid);
		oidclr(&b->branch_tree.versions[0].oid);
		oidclr(&b->branch_tree.versions[1].oid);
		if (b->branch_tree.tree) {
			release_tree_content_recursive(b->branch_tree.tree);
			b->branch_tree.tree = NULL;
		}
	}
	else
		b = new_branch(arg);
	read_next_command();
	parse_from(b);
	if (command_buf.len > 0)
		unread_command_buf = 1;
}