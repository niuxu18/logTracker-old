 */
#include "cache.h"
#include "diff.h"
#include "diffcore.h"
#include "tree.h"

/*
 * internal mode marker, saying a tree entry != entry of tp[imin]
 * (see ll_diff_tree_paths for what it means there)
 *
 * we will update/use/emit entry for diff only with it unset.
 */
#define S_IFXMIN_NEQ	S_DIFFTREE_IFXMIN_NEQ


static struct combine_diff_path *ll_diff_tree_paths(
	struct combine_diff_path *p, const unsigned char *sha1,
	const unsigned char **parents_sha1, int nparent,
	struct strbuf *base, struct diff_options *opt);
static int ll_diff_tree_sha1(const unsigned char *old, const unsigned char *new,
			     struct strbuf *base, struct diff_options *opt);

/*
 * Compare two tree entries, taking into account only path/S_ISDIR(mode),
 * but not their sha1's.
 *
 * NOTE files and directories *always* compare differently, even when having
 *      the same name - thanks to base_name_compare().
 *
 * NOTE empty (=invalid) descriptor(s) take part in comparison as +infty,
 *      so that they sort *after* valid tree entries.
 *
 *      Due to this convention, if trees are scanned in sorted order, all
 *      non-empty descriptors will be processed first.
 */
static int tree_entry_pathcmp(struct tree_desc *t1, struct tree_desc *t2)
{
	struct name_entry *e1, *e2;
	int cmp;

	/* empty descriptors sort after valid tree entries */
	if (!t1->size)
		return t2->size ? 1 : 0;
	else if (!t2->size)
		return -1;

	e1 = &t1->entry;
	e2 = &t2->entry;
	cmp = base_name_compare(e1->path, tree_entry_len(e1), e1->mode,
				e2->path, tree_entry_len(e2), e2->mode);
	return cmp;
}


/*
 * convert path -> opt->diff_*() callbacks
 *
 * emits diff to first parent only, and tells diff tree-walker that we are done
 * with p and it can be freed.
 */
static int emit_diff_first_parent_only(struct diff_options *opt, struct combine_diff_path *p)
{
	struct combine_diff_parent *p0 = &p->parent[0];
	if (p->mode && p0->mode) {
		opt->change(opt, p0->mode, p->mode, p0->sha1, p->sha1,
			1, 1, p->path, 0, 0);
	}
	else {
		const unsigned char *sha1;
		unsigned int mode;
		int addremove;

		if (p->mode) {
			addremove = '+';
			sha1 = p->sha1;
			mode = p->mode;
		} else {
			addremove = '-';
			sha1 = p0->sha1;
			mode = p0->mode;
		}

		opt->add_remove(opt, addremove, mode, sha1, 1, p->path, 0);
	}

	return 0;	/* we are done with p */
}


/*
 * Make a new combine_diff_path from path/mode/sha1
 * and append it to paths list tail.
 *
 * Memory for created elements could be reused:
 *
 *	- if last->next == NULL, the memory is allocated;
 *
 *	- if last->next != NULL, it is assumed that p=last->next was returned
 *	  earlier by this function, and p->next was *not* modified.
 *	  The memory is then reused from p.
 *
 * so for clients,
 *
 * - if you do need to keep the element
 *
 *	p = path_appendnew(p, ...);
 *	process(p);
 *	p->next = NULL;
 *
 * - if you don't need to keep the element after processing
 *
 *	pprev = p;
 *	p = path_appendnew(p, ...);
 *	process(p);
 *	p = pprev;
 *	; don't forget to free tail->next in the end
 *
 * p->parent[] remains uninitialized.
 */
static struct combine_diff_path *path_appendnew(struct combine_diff_path *last,
	int nparent, const struct strbuf *base, const char *path, int pathlen,
	unsigned mode, const unsigned char *sha1)
{
	struct combine_diff_path *p;
	int len = base->len + pathlen;
	int alloclen = combine_diff_path_size(nparent, len);

	/* if last->next is !NULL - it is a pre-allocated memory, we can reuse */
	p = last->next;
	if (p && (alloclen > (intptr_t)p->next)) {
		free(p);
		p = NULL;
	}

	if (!p) {
		p = xmalloc(alloclen);

		/*
		 * until we go to it next round, .next holds how many bytes we
		 * allocated (for faster realloc - we don't need copying old data).
		 */
		p->next = (struct combine_diff_path *)(intptr_t)alloclen;
	}

	last->next = p;

	p->path = (char *)&(p->parent[nparent]);
	memcpy(p->path, base->buf, base->len);
	memcpy(p->path + base->len, path, pathlen);
	p->path[len] = 0;
	p->mode = mode;
	hashcpy(p->sha1, sha1 ? sha1 : null_sha1);

	return p;
}

/*
 * new path should be added to combine diff
 *
 * 3 cases on how/when it should be called and behaves:
 *
 *	 t, !tp		-> path added, all parents lack it
 *	!t,  tp		-> path removed from all parents
 *	 t,  tp		-> path modified/added
 *			   (M for tp[i]=tp[imin], A otherwise)
 */
static struct combine_diff_path *emit_path(struct combine_diff_path *p,
	struct strbuf *base, struct diff_options *opt, int nparent,
	struct tree_desc *t, struct tree_desc *tp,
	int imin)
{
	unsigned mode;
	const char *path;
	const unsigned char *sha1;
	int pathlen;
	int old_baselen = base->len;
	int i, isdir, recurse = 0, emitthis = 1;

	/* at least something has to be valid */
	assert(t || tp);

	if (t) {
		/* path present in resulting tree */
		sha1 = tree_entry_extract(t, &path, &mode);
		pathlen = tree_entry_len(&t->entry);
		isdir = S_ISDIR(mode);
	} else {
		/*
		 * a path was removed - take path from imin parent. Also take
		 * mode from that parent, to decide on recursion(1).
		 *
		 * 1) all modes for tp[i]=tp[imin] should be the same wrt
		 *    S_ISDIR, thanks to base_name_compare().
		 */
		tree_entry_extract(&tp[imin], &path, &mode);
		pathlen = tree_entry_len(&tp[imin].entry);

		isdir = S_ISDIR(mode);
		sha1 = NULL;
		mode = 0;
	}

	if (DIFF_OPT_TST(opt, RECURSIVE) && isdir) {
		recurse = 1;
		emitthis = DIFF_OPT_TST(opt, TREE_IN_RECURSIVE);
	}

	if (emitthis) {
		int keep;
		struct combine_diff_path *pprev = p;
		p = path_appendnew(p, nparent, base, path, pathlen, mode, sha1);

		for (i = 0; i < nparent; ++i) {
			/*
			 * tp[i] is valid, if present and if tp[i]==tp[imin] -
			 * otherwise, we should ignore it.
			 */
			int tpi_valid = tp && !(tp[i].entry.mode & S_IFXMIN_NEQ);

			const unsigned char *sha1_i;
			unsigned mode_i;

			p->parent[i].status =
				!t ? DIFF_STATUS_DELETED :
					tpi_valid ?
						DIFF_STATUS_MODIFIED :
						DIFF_STATUS_ADDED;

			if (tpi_valid) {
				sha1_i = tp[i].entry.sha1;
				mode_i = tp[i].entry.mode;
			}
			else {
				sha1_i = NULL;
				mode_i = 0;
			}

			p->parent[i].mode = mode_i;
			hashcpy(p->parent[i].sha1, sha1_i ? sha1_i : null_sha1);
		}

		keep = 1;
		if (opt->pathchange)
			keep = opt->pathchange(opt, p);

		/*
		 * If a path was filtered or consumed - we don't need to add it
		 * to the list and can reuse its memory, leaving it as
		 * pre-allocated element on the tail.
		 *
		 * On the other hand, if path needs to be kept, we need to
		 * correct its .next to NULL, as it was pre-initialized to how
		 * much memory was allocated.
		 *
		 * see path_appendnew() for details.
		 */
		if (!keep)
			p = pprev;
		else
			p->next = NULL;
	}

	if (recurse) {
		const unsigned char **parents_sha1;

		parents_sha1 = xalloca(nparent * sizeof(parents_sha1[0]));
		for (i = 0; i < nparent; ++i) {
			/* same rule as in emitthis */
			int tpi_valid = tp && !(tp[i].entry.mode & S_IFXMIN_NEQ);

			parents_sha1[i] = tpi_valid ? tp[i].entry.sha1
						    : NULL;
		}

		strbuf_add(base, path, pathlen);
		strbuf_addch(base, '/');
		p = ll_diff_tree_paths(p, sha1, parents_sha1, nparent, base, opt);
		xalloca_free(parents_sha1);
	}

	strbuf_setlen(base, old_baselen);
	return p;
}

static void skip_uninteresting(struct tree_desc *t, struct strbuf *base,
			       struct diff_options *opt)
{
	enum interesting match;

	while (t->size) {
		match = tree_entry_interesting(&t->entry, base, 0, &opt->pathspec);
		if (match) {
			if (match == all_entries_not_interesting)
				t->size = 0;
			break;
		}
		update_tree_entry(t);
	}
}


/*
 * generate paths for combined diff D(sha1,parents_sha1[])
 *
 * Resulting paths are appended to combine_diff_path linked list, and also, are
 * emitted on the go via opt->pathchange() callback, so it is possible to
 * process the result as batch or incrementally.
 *
 * The paths are generated scanning new tree and all parents trees
 * simultaneously, similarly to what diff_tree() was doing for 2 trees.
 * The theory behind such scan is as follows:
 *
 *
 * D(T,P1...Pn) calculation scheme
 * -------------------------------
 *
 * D(T,P1...Pn) = D(T,P1) ^ ... ^ D(T,Pn)	(regarding resulting paths set)
 *
 *	D(T,Pj)		- diff between T..Pj
 *	D(T,P1...Pn)	- combined diff from T to parents P1,...,Pn
 *
 *
 * We start from all trees, which are sorted, and compare their entries in
 * lock-step:
 *
 *	 T     P1       Pn
 *	 -     -        -
 *	|t|   |p1|     |pn|
 *	|-|   |--| ... |--|      imin = argmin(p1...pn)
 *	| |   |  |     |  |
 *	|-|   |--|     |--|
 *	|.|   |. |     |. |
 *	 .     .        .
 *	 .     .        .
 *
 * at any time there could be 3 cases:
 *
 *	1)  t < p[imin];
 *	2)  t > p[imin];
 *	3)  t = p[imin].
 *
 * Schematic deduction of what every case means, and what to do, follows:
 *
 * 1)  t < p[imin]  ->  ∀j t ∉ Pj  ->  "+t" ∈ D(T,Pj)  ->  D += "+t";  t↓
 *
 * 2)  t > p[imin]
 *
 *     2.1) ∃j: pj > p[imin]  ->  "-p[imin]" ∉ D(T,Pj)  ->  D += ø;  ∀ pi=p[imin]  pi↓
 *     2.2) ∀i  pi = p[imin]  ->  pi ∉ T  ->  "-pi" ∈ D(T,Pi)  ->  D += "-p[imin]";  ∀i pi↓
 *
 * 3)  t = p[imin]
 *
 *     3.1) ∃j: pj > p[imin]  ->  "+t" ∈ D(T,Pj)  ->  only pi=p[imin] remains to investigate
 *     3.2) pi = p[imin]  ->  investigate δ(t,pi)
 *      |
 *      |
 *      v
 *
 *     3.1+3.2) looking at δ(t,pi) ∀i: pi=p[imin] - if all != ø  ->
 *
 *                       ⎧δ(t,pi)  - if pi=p[imin]
 *              ->  D += ⎨
 *                       ⎩"+t"     - if pi>p[imin]
 *
 *
 *     in any case t↓  ∀ pi=p[imin]  pi↓
 *
 *
 * ~~~~~~~~
 *
 * NOTE
 *
 *	Usual diff D(A,B) is by definition the same as combined diff D(A,[B]),
 *	so this diff paths generator can, and is used, for plain diffs
 *	generation too.
 *
 *	Please keep attention to the common D(A,[B]) case when working on the
 *	code, in order not to slow it down.
 *
 * NOTE
 *	nparent must be > 0.
 */


/* ∀ pi=p[imin]  pi↓ */
static inline void update_tp_entries(struct tree_desc *tp, int nparent)
{
	int i;
	for (i = 0; i < nparent; ++i)
		if (!(tp[i].entry.mode & S_IFXMIN_NEQ))
			update_tree_entry(&tp[i]);
}

static struct combine_diff_path *ll_diff_tree_paths(
	struct combine_diff_path *p, const unsigned char *sha1,
	const unsigned char **parents_sha1, int nparent,
	struct strbuf *base, struct diff_options *opt)
{
	struct tree_desc t, *tp;
	void *ttree, **tptree;
	int i;

	tp     = xalloca(nparent * sizeof(tp[0]));
	tptree = xalloca(nparent * sizeof(tptree[0]));

	/*
	 * load parents first, as they are probably already cached.
	 *
	 * ( log_tree_diff() parses commit->parent before calling here via
	 *   diff_tree_sha1(parent, commit) )
	 */
	for (i = 0; i < nparent; ++i)
		tptree[i] = fill_tree_descriptor(&tp[i], parents_sha1[i]);
	ttree = fill_tree_descriptor(&t, sha1);

	/* Enable recursion indefinitely */
	opt->pathspec.recursive = DIFF_OPT_TST(opt, RECURSIVE);

	for (;;) {
		int imin, cmp;

		if (diff_can_quit_early(opt))
			break;

		if (opt->pathspec.nr) {
			skip_uninteresting(&t, base, opt);
			for (i = 0; i < nparent; i++)
				skip_uninteresting(&tp[i], base, opt);
		}

		/* comparing is finished when all trees are done */
		if (!t.size) {
			int done = 1;
			for (i = 0; i < nparent; ++i)
				if (tp[i].size) {
					done = 0;
					break;
				}
			if (done)
				break;
		}

		/*
		 * lookup imin = argmin(p1...pn),
		 * mark entries whether they =p[imin] along the way
		 */
		imin = 0;
		tp[0].entry.mode &= ~S_IFXMIN_NEQ;

		for (i = 1; i < nparent; ++i) {
			cmp = tree_entry_pathcmp(&tp[i], &tp[imin]);
			if (cmp < 0) {
				imin = i;
				tp[i].entry.mode &= ~S_IFXMIN_NEQ;
			}
			else if (cmp == 0) {
				tp[i].entry.mode &= ~S_IFXMIN_NEQ;
			}
			else {
				tp[i].entry.mode |= S_IFXMIN_NEQ;
			}
		}

		/* fixup markings for entries before imin */
		for (i = 0; i < imin; ++i)
			tp[i].entry.mode |= S_IFXMIN_NEQ;	/* pi > p[imin] */



		/* compare t vs p[imin] */
		cmp = tree_entry_pathcmp(&t, &tp[imin]);

		/* t = p[imin] */
		if (cmp == 0) {
			/* are either pi > p[imin] or diff(t,pi) != ø ? */
			if (!DIFF_OPT_TST(opt, FIND_COPIES_HARDER)) {
				for (i = 0; i < nparent; ++i) {
					/* p[i] > p[imin] */
					if (tp[i].entry.mode & S_IFXMIN_NEQ)
						continue;

					/* diff(t,pi) != ø */
					if (hashcmp(t.entry.sha1, tp[i].entry.sha1) ||
					    (t.entry.mode != tp[i].entry.mode))
						continue;

					goto skip_emit_t_tp;
				}
			}

			/* D += {δ(t,pi) if pi=p[imin];  "+a" if pi > p[imin]} */
			p = emit_path(p, base, opt, nparent,
					&t, tp, imin);

		skip_emit_t_tp:
			/* t↓,  ∀ pi=p[imin]  pi↓ */
			update_tree_entry(&t);
			update_tp_entries(tp, nparent);
		}

		/* t < p[imin] */
		else if (cmp < 0) {
			/* D += "+t" */
			p = emit_path(p, base, opt, nparent,
					&t, /*tp=*/NULL, -1);

			/* t↓ */
			update_tree_entry(&t);
		}

		/* t > p[imin] */
		else {
			/* ∀i pi=p[imin] -> D += "-p[imin]" */
			if (!DIFF_OPT_TST(opt, FIND_COPIES_HARDER)) {
				for (i = 0; i < nparent; ++i)
					if (tp[i].entry.mode & S_IFXMIN_NEQ)
						goto skip_emit_tp;
			}

			p = emit_path(p, base, opt, nparent,
					/*t=*/NULL, tp, imin);

		skip_emit_tp:
			/* ∀ pi=p[imin]  pi↓ */
			update_tp_entries(tp, nparent);
		}
	}

	free(ttree);
	for (i = nparent-1; i >= 0; i--)
		free(tptree[i]);
	xalloca_free(tptree);
	xalloca_free(tp);

	return p;
}

struct combine_diff_path *diff_tree_paths(
	struct combine_diff_path *p, const unsigned char *sha1,
	const unsigned char **parents_sha1, int nparent,
	struct strbuf *base, struct diff_options *opt)
{
	p = ll_diff_tree_paths(p, sha1, parents_sha1, nparent, base, opt);

	/*
	 * free pre-allocated last element, if any
	 * (see path_appendnew() for details about why)
	 */
	if (p->next) {
		free(p->next);
		p->next = NULL;
	}

	return p;
}

/*
 * Does it look like the resulting diff might be due to a rename?
 *  - single entry
 *  - not a valid previous file
