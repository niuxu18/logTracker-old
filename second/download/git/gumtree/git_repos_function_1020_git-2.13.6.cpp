static int split_commit_in_progress(struct wt_status *s)
{
	int split_in_progress = 0;
	char *head, *orig_head, *rebase_amend, *rebase_orig_head;

	if ((!s->amend && !s->nowarn && !s->workdir_dirty) ||
	    !s->branch || strcmp(s->branch, "HEAD"))
		return 0;

	head = read_line_from_git_path("HEAD");
	orig_head = read_line_from_git_path("ORIG_HEAD");
	rebase_amend = read_line_from_git_path("rebase-merge/amend");
	rebase_orig_head = read_line_from_git_path("rebase-merge/orig-head");

	if (!head || !orig_head || !rebase_amend || !rebase_orig_head)
		; /* fall through, no split in progress */
	else if (!strcmp(rebase_amend, rebase_orig_head))
		split_in_progress = !!strcmp(head, rebase_amend);
	else if (strcmp(orig_head, rebase_orig_head))
		split_in_progress = 1;

	free(head);
	free(orig_head);
	free(rebase_amend);
	free(rebase_orig_head);

	return split_in_progress;
}