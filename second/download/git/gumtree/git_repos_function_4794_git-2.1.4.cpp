static int checkout(void)
{
	unsigned char sha1[20];
	char *head;
	struct lock_file *lock_file;
	struct unpack_trees_options opts;
	struct tree *tree;
	struct tree_desc t;
	int err = 0;

	if (option_no_checkout)
		return 0;

	head = resolve_refdup("HEAD", sha1, 1, NULL);
	if (!head) {
		warning(_("remote HEAD refers to nonexistent ref, "
			  "unable to checkout.\n"));
		return 0;
	}
	if (!strcmp(head, "HEAD")) {
		if (advice_detached_head)
			detach_advice(sha1_to_hex(sha1));
	} else {
		if (!starts_with(head, "refs/heads/"))
			die(_("HEAD not found below refs/heads!"));
	}
	free(head);

	/* We need to be in the new work tree for the checkout */
	setup_work_tree();

	lock_file = xcalloc(1, sizeof(struct lock_file));
	hold_locked_index(lock_file, 1);

	memset(&opts, 0, sizeof opts);
	opts.update = 1;
	opts.merge = 1;
	opts.fn = oneway_merge;
	opts.verbose_update = (option_verbosity >= 0);
	opts.src_index = &the_index;
	opts.dst_index = &the_index;

	tree = parse_tree_indirect(sha1);
	parse_tree(tree);
	init_tree_desc(&t, tree->buffer, tree->size);
	if (unpack_trees(1, &t, &opts) < 0)
		die(_("unable to checkout working tree"));

	if (write_locked_index(&the_index, lock_file, COMMIT_LOCK))
		die(_("unable to write new index file"));

	err |= run_hook_le(NULL, "post-checkout", sha1_to_hex(null_sha1),
			   sha1_to_hex(sha1), "1", NULL);

	if (!err && option_recursive)
		err = run_command_v_opt(argv_submodule, RUN_GIT_CMD);

	return err;
}