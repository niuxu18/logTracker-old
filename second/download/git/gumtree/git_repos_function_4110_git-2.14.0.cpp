static void threaded_lazy_init_name_hash(
	struct index_state *istate)
{
	int nr_each;
	int k_start;
	int t;
	struct lazy_entry *lazy_entries;
	struct lazy_dir_thread_data *td_dir;
	struct lazy_name_thread_data *td_name;

	k_start = 0;
	nr_each = DIV_ROUND_UP(istate->cache_nr, lazy_nr_dir_threads);

	lazy_entries = xcalloc(istate->cache_nr, sizeof(struct lazy_entry));
	td_dir = xcalloc(lazy_nr_dir_threads, sizeof(struct lazy_dir_thread_data));
	td_name = xcalloc(1, sizeof(struct lazy_name_thread_data));

	init_dir_mutex();

	/*
	 * Phase 1:
	 * Build "istate->dir_hash" using n "dir" threads (and a read-only index).
	 */
	for (t = 0; t < lazy_nr_dir_threads; t++) {
		struct lazy_dir_thread_data *td_dir_t = td_dir + t;
		td_dir_t->istate = istate;
		td_dir_t->lazy_entries = lazy_entries;
		td_dir_t->k_start = k_start;
		k_start += nr_each;
		if (k_start > istate->cache_nr)
			k_start = istate->cache_nr;
		td_dir_t->k_end = k_start;
		if (pthread_create(&td_dir_t->pthread, NULL, lazy_dir_thread_proc, td_dir_t))
			die("unable to create lazy_dir_thread");
	}
	for (t = 0; t < lazy_nr_dir_threads; t++) {
		struct lazy_dir_thread_data *td_dir_t = td_dir + t;
		if (pthread_join(td_dir_t->pthread, NULL))
			die("unable to join lazy_dir_thread");
	}

	/*
	 * Phase 2:
	 * Iterate over all index entries and add them to the "istate->name_hash"
	 * using a single "name" background thread.
	 * (Testing showed it wasn't worth running more than 1 thread for this.)
	 *
	 * Meanwhile, finish updating the parent directory ref-counts for each
	 * index entry using the current thread.  (This step is very fast and
	 * doesn't need threading.)
	 */
	td_name->istate = istate;
	td_name->lazy_entries = lazy_entries;
	if (pthread_create(&td_name->pthread, NULL, lazy_name_thread_proc, td_name))
		die("unable to create lazy_name_thread");

	lazy_update_dir_ref_counts(istate, lazy_entries);

	if (pthread_join(td_name->pthread, NULL))
		die("unable to join lazy_name_thread");

	cleanup_dir_mutex();

	free(td_name);
	free(td_dir);
	free(lazy_entries);
}