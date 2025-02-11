static void prepare_attr_stack(const char *path, int dirlen,
			       struct attr_stack **stack)
{
	struct attr_stack *info;
	struct strbuf pathbuf = STRBUF_INIT;

	/*
	 * At the bottom of the attribute stack is the built-in
	 * set of attribute definitions, followed by the contents
	 * of $(prefix)/etc/gitattributes and a file specified by
	 * core.attributesfile.  Then, contents from
	 * .gitattribute files from directories closer to the
	 * root to the ones in deeper directories are pushed
	 * to the stack.  Finally, at the very top of the stack
	 * we always keep the contents of $GIT_DIR/info/attributes.
	 *
	 * When checking, we use entries from near the top of the
	 * stack, preferring $GIT_DIR/info/attributes, then
	 * .gitattributes in deeper directories to shallower ones,
	 * and finally use the built-in set as the default.
	 */
	bootstrap_attr_stack(stack);

	/*
	 * Pop the "info" one that is always at the top of the stack.
	 */
	info = *stack;
	*stack = info->prev;

	/*
	 * Pop the ones from directories that are not the prefix of
	 * the path we are checking. Break out of the loop when we see
	 * the root one (whose origin is an empty string "") or the builtin
	 * one (whose origin is NULL) without popping it.
	 */
	while ((*stack)->origin) {
		int namelen = (*stack)->originlen;
		struct attr_stack *elem;

		elem = *stack;
		if (namelen <= dirlen &&
		    !strncmp(elem->origin, path, namelen) &&
		    (!namelen || path[namelen] == '/'))
			break;

		debug_pop(elem);
		*stack = elem->prev;
		attr_stack_free(elem);
	}

	/*
	 * bootstrap_attr_stack() should have added, and the
	 * above loop should have stopped before popping, the
	 * root element whose attr_stack->origin is set to an
	 * empty string.
	 */
	assert((*stack)->origin);

	strbuf_addstr(&pathbuf, (*stack)->origin);
	/* Build up to the directory 'path' is in */
	while (pathbuf.len < dirlen) {
		size_t len = pathbuf.len;
		struct attr_stack *next;
		char *origin;

		/* Skip path-separator */
		if (len < dirlen && is_dir_sep(path[len]))
			len++;
		/* Find the end of the next component */
		while (len < dirlen && !is_dir_sep(path[len]))
			len++;

		if (pathbuf.len > 0)
			strbuf_addch(&pathbuf, '/');
		strbuf_add(&pathbuf, path + pathbuf.len, (len - pathbuf.len));
		strbuf_addf(&pathbuf, "/%s", GITATTRIBUTES_FILE);

		next = read_attr(pathbuf.buf, 0);

		/* reset the pathbuf to not include "/.gitattributes" */
		strbuf_setlen(&pathbuf, len);

		origin = xstrdup(pathbuf.buf);
		push_stack(stack, next, origin, len);
	}

	/*
	 * Finally push the "info" one at the top of the stack.
	 */
	push_stack(stack, info, NULL, 0);

	strbuf_release(&pathbuf);
}