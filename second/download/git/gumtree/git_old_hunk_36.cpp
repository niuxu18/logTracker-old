	 * commit message and parents from 'partial_commit'.
	 * Finally store the new commit object SHA1 into 'result_sha1'.
	 */
	DIR *dir;
	struct dirent *e;
	struct strbuf path = STRBUF_INIT;
	char *msg = strstr(partial_commit->buffer, "\n\n");
	struct strbuf sb_msg = STRBUF_INIT;
	int baselen;

	strbuf_addstr(&path, git_path(NOTES_MERGE_WORKTREE));
	if (o->verbosity >= 3)
		printf("Committing notes in notes merge worktree at %s\n",
			path.buf);
