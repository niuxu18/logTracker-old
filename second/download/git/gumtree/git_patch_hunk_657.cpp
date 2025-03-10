 			      struct branch_info *new,
 			      int *writeout_error)
 {
 	int ret;
 	struct lock_file *lock_file = xcalloc(1, sizeof(struct lock_file));
 
-	hold_locked_index(lock_file, 1);
+	hold_locked_index(lock_file, LOCK_DIE_ON_ERROR);
 	if (read_cache_preload(NULL) < 0)
 		return error(_("index file corrupt"));
 
 	resolve_undo_clear();
 	if (opts->force) {
 		ret = reset_tree(new->commit->tree, opts, 1, writeout_error);
