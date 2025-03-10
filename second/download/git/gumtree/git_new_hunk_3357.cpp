}

static int merge_trivial(struct commit *head, struct commit_list *remoteheads)
{
	unsigned char result_tree[20], result_commit[20];
	struct commit_list *parents, **pptr = &parents;
	static struct lock_file lock;

	hold_locked_index(&lock, 1);
	refresh_cache(REFRESH_QUIET);
	if (active_cache_changed &&
	    write_locked_index(&the_index, &lock, COMMIT_LOCK))
		return error(_("Unable to write index."));
	rollback_lock_file(&lock);

	write_tree_trivial(result_tree);
	printf(_("Wonderful.\n"));
	pptr = commit_list_append(head, pptr);
	pptr = commit_list_append(remoteheads->item, pptr);
	prepare_to_commit(remoteheads);
