		error("%s does not point to a valid object!", refname);
		return 0;
	}
	return 1;
}

/*
 * Return true if the reference described by entry can be resolved to
 * an object in the database; otherwise, emit a warning and return
 * false.
 */
static int entry_resolves_to_object(struct ref_entry *entry)
{
	return ref_resolves_to_object(entry->name,
				      &entry->u.value.oid, entry->flag);
}

typedef int each_ref_entry_fn(struct ref_entry *entry, void *cb_data);

/*
 * Call fn for each reference in dir that has index in the range
 * offset <= index < dir->nr.  Recurse into subdirectories that are in
 * that index range, sorting them before iterating.  This function
 * does not sort dir itself; it should be sorted beforehand.  fn is
 * called for all references, including broken ones.
 */
static int do_for_each_entry_in_dir(struct ref_dir *dir, int offset,
				    each_ref_entry_fn fn, void *cb_data)
{
	int i;
	assert(dir->sorted == dir->nr);
	for (i = offset; i < dir->nr; i++) {
		struct ref_entry *entry = dir->entries[i];
		int retval;
		if (entry->flag & REF_DIR) {
			struct ref_dir *subdir = get_ref_dir(entry);
			sort_ref_dir(subdir);
			retval = do_for_each_entry_in_dir(subdir, 0, fn, cb_data);
		} else {
			retval = fn(entry, cb_data);
		}
		if (retval)
			return retval;
	}
	return 0;
}

/*
 * Load all of the refs from the dir into our in-memory cache. The hard work
 * of loading loose refs is done by get_ref_dir(), so we just need to recurse
 * through all of the sub-directories. We do not even need to care about
 * sorting, as traversal order does not matter to us.
 */
static void prime_ref_dir(struct ref_dir *dir)
{
	int i;
	for (i = 0; i < dir->nr; i++) {
		struct ref_entry *entry = dir->entries[i];
		if (entry->flag & REF_DIR)
			prime_ref_dir(get_ref_dir(entry));
	}
}

/*
 * A level in the reference hierarchy that is currently being iterated
 * through.
 */
struct cache_ref_iterator_level {
	/*
	 * The ref_dir being iterated over at this level. The ref_dir
	 * is sorted before being stored here.
	 */
	struct ref_dir *dir;

	/*
	 * The index of the current entry within dir (which might
	 * itself be a directory). If index == -1, then the iteration
	 * hasn't yet begun. If index == dir->nr, then the iteration
	 * through this level is over.
	 */
	int index;
};

/*
 * Represent an iteration through a ref_dir in the memory cache. The
 * iteration recurses through subdirectories.
 */
struct cache_ref_iterator {
	struct ref_iterator base;

	/*
	 * The number of levels currently on the stack. This is always
	 * at least 1, because when it becomes zero the iteration is
	 * ended and this struct is freed.
	 */
	size_t levels_nr;

	/* The number of levels that have been allocated on the stack */
	size_t levels_alloc;

	/*
	 * A stack of levels. levels[0] is the uppermost level that is
	 * being iterated over in this iteration. (This is not
	 * necessary the top level in the references hierarchy. If we
	 * are iterating through a subtree, then levels[0] will hold
	 * the ref_dir for that subtree, and subsequent levels will go
	 * on from there.)
	 */
	struct cache_ref_iterator_level *levels;
};

static int cache_ref_iterator_advance(struct ref_iterator *ref_iterator)
{
	struct cache_ref_iterator *iter =
		(struct cache_ref_iterator *)ref_iterator;

	while (1) {
		struct cache_ref_iterator_level *level =
			&iter->levels[iter->levels_nr - 1];
		struct ref_dir *dir = level->dir;
		struct ref_entry *entry;

		if (level->index == -1)
			sort_ref_dir(dir);

		if (++level->index == level->dir->nr) {
			/* This level is exhausted; pop up a level */
			if (--iter->levels_nr == 0)
				return ref_iterator_abort(ref_iterator);

			continue;
		}

		entry = dir->entries[level->index];

		if (entry->flag & REF_DIR) {
			/* push down a level */
			ALLOC_GROW(iter->levels, iter->levels_nr + 1,
				   iter->levels_alloc);

			level = &iter->levels[iter->levels_nr++];
			level->dir = get_ref_dir(entry);
			level->index = -1;
		} else {
			iter->base.refname = entry->name;
			iter->base.oid = &entry->u.value.oid;
			iter->base.flags = entry->flag;
			return ITER_OK;
		}
	}
}

static enum peel_status peel_entry(struct ref_entry *entry, int repeel);

static int cache_ref_iterator_peel(struct ref_iterator *ref_iterator,
				   struct object_id *peeled)
{
	struct cache_ref_iterator *iter =
		(struct cache_ref_iterator *)ref_iterator;
	struct cache_ref_iterator_level *level;
	struct ref_entry *entry;

	level = &iter->levels[iter->levels_nr - 1];

	if (level->index == -1)
		die("BUG: peel called before advance for cache iterator");

	entry = level->dir->entries[level->index];

	if (peel_entry(entry, 0))
		return -1;
	oidcpy(peeled, &entry->u.value.peeled);
	return 0;
}

static int cache_ref_iterator_abort(struct ref_iterator *ref_iterator)
{
	struct cache_ref_iterator *iter =
		(struct cache_ref_iterator *)ref_iterator;

	free(iter->levels);
	base_ref_iterator_free(ref_iterator);
	return ITER_DONE;
}

static struct ref_iterator_vtable cache_ref_iterator_vtable = {
	cache_ref_iterator_advance,
	cache_ref_iterator_peel,
	cache_ref_iterator_abort
};

static struct ref_iterator *cache_ref_iterator_begin(struct ref_dir *dir)
{
	struct cache_ref_iterator *iter;
	struct ref_iterator *ref_iterator;
	struct cache_ref_iterator_level *level;

	iter = xcalloc(1, sizeof(*iter));
	ref_iterator = &iter->base;
	base_ref_iterator_init(ref_iterator, &cache_ref_iterator_vtable);
	ALLOC_GROW(iter->levels, 10, iter->levels_alloc);

	iter->levels_nr = 1;
	level = &iter->levels[0];
	level->index = -1;
	level->dir = dir;

	return ref_iterator;
}

struct nonmatching_ref_data {
	const struct string_list *skip;
	const char *conflicting_refname;
};

static int nonmatching_ref_fn(struct ref_entry *entry, void *vdata)
{
	struct nonmatching_ref_data *data = vdata;

	if (data->skip && string_list_has_string(data->skip, entry->name))
		return 0;

	data->conflicting_refname = entry->name;
	return 1;
}

/*
 * Return 0 if a reference named refname could be created without
 * conflicting with the name of an existing reference in dir.
 * See verify_refname_available for more information.
 */
static int verify_refname_available_dir(const char *refname,
					const struct string_list *extras,
					const struct string_list *skip,
					struct ref_dir *dir,
					struct strbuf *err)
{
	const char *slash;
	const char *extra_refname;
	int pos;
	struct strbuf dirname = STRBUF_INIT;
	int ret = -1;

	/*
	 * For the sake of comments in this function, suppose that
	 * refname is "refs/foo/bar".
	 */

	assert(err);

	strbuf_grow(&dirname, strlen(refname) + 1);
	for (slash = strchr(refname, '/'); slash; slash = strchr(slash + 1, '/')) {
		/* Expand dirname to the new prefix, not including the trailing slash: */
		strbuf_add(&dirname, refname + dirname.len, slash - refname - dirname.len);

		/*
		 * We are still at a leading dir of the refname (e.g.,
		 * "refs/foo"; if there is a reference with that name,
		 * it is a conflict, *unless* it is in skip.
		 */
		if (dir) {
			pos = search_ref_dir(dir, dirname.buf, dirname.len);
			if (pos >= 0 &&
			    (!skip || !string_list_has_string(skip, dirname.buf))) {
				/*
				 * We found a reference whose name is
				 * a proper prefix of refname; e.g.,
				 * "refs/foo", and is not in skip.
				 */
				strbuf_addf(err, "'%s' exists; cannot create '%s'",
					    dirname.buf, refname);
				goto cleanup;
			}
		}

		if (extras && string_list_has_string(extras, dirname.buf) &&
		    (!skip || !string_list_has_string(skip, dirname.buf))) {
			strbuf_addf(err, "cannot process '%s' and '%s' at the same time",
				    refname, dirname.buf);
			goto cleanup;
		}

		/*
		 * Otherwise, we can try to continue our search with
		 * the next component. So try to look up the
		 * directory, e.g., "refs/foo/". If we come up empty,
		 * we know there is nothing under this whole prefix,
		 * but even in that case we still have to continue the
		 * search for conflicts with extras.
		 */
		strbuf_addch(&dirname, '/');
		if (dir) {
			pos = search_ref_dir(dir, dirname.buf, dirname.len);
			if (pos < 0) {
				/*
				 * There was no directory "refs/foo/",
				 * so there is nothing under this
				 * whole prefix. So there is no need
				 * to continue looking for conflicting
				 * references. But we need to continue
				 * looking for conflicting extras.
				 */
				dir = NULL;
			} else {
				dir = get_ref_dir(dir->entries[pos]);
			}
		}
	}

	/*
	 * We are at the leaf of our refname (e.g., "refs/foo/bar").
	 * There is no point in searching for a reference with that
	 * name, because a refname isn't considered to conflict with
	 * itself. But we still need to check for references whose
	 * names are in the "refs/foo/bar/" namespace, because they
	 * *do* conflict.
	 */
	strbuf_addstr(&dirname, refname + dirname.len);
	strbuf_addch(&dirname, '/');

	if (dir) {
		pos = search_ref_dir(dir, dirname.buf, dirname.len);

		if (pos >= 0) {
			/*
			 * We found a directory named "$refname/"
			 * (e.g., "refs/foo/bar/"). It is a problem
			 * iff it contains any ref that is not in
			 * "skip".
			 */
			struct nonmatching_ref_data data;

			data.skip = skip;
			data.conflicting_refname = NULL;
			dir = get_ref_dir(dir->entries[pos]);
			sort_ref_dir(dir);
			if (do_for_each_entry_in_dir(dir, 0, nonmatching_ref_fn, &data)) {
				strbuf_addf(err, "'%s' exists; cannot create '%s'",
					    data.conflicting_refname, refname);
				goto cleanup;
			}
		}
	}

	extra_refname = find_descendant_ref(dirname.buf, extras, skip);
	if (extra_refname)
		strbuf_addf(err, "cannot process '%s' and '%s' at the same time",
			    refname, extra_refname);
	else
		ret = 0;

cleanup:
	strbuf_release(&dirname);
	return ret;
}

struct packed_ref_cache {
	struct ref_entry *root;

	/*
	 * Count of references to the data structure in this instance,
	 * including the pointer from files_ref_store::packed if any.
	 * The data will not be freed as long as the reference count
	 * is nonzero.
