static struct commit *handle_commit(struct rev_info *revs,
				    struct object *object, const char *name)
{
	unsigned long flags = object->flags;

	/*
	 * Tag object? Look what it points to..
	 */
	while (object->type == OBJ_TAG) {
		struct tag *tag = (struct tag *) object;
		if (revs->tag_objects && !(flags & UNINTERESTING))
			add_pending_object(revs, object, tag->tag);
		if (!tag->tagged)
			die("bad tag");
		object = parse_object(tag->tagged->sha1);
		if (!object) {
			if (flags & UNINTERESTING)
				return NULL;
			die("bad object %s", sha1_to_hex(tag->tagged->sha1));
		}
		object->flags |= flags;
	}

	/*
	 * Commit object? Just return it, we'll do all the complex
	 * reachability crud.
	 */
	if (object->type == OBJ_COMMIT) {
		struct commit *commit = (struct commit *)object;
		if (parse_commit(commit) < 0)
			die("unable to parse commit %s", name);
		if (flags & UNINTERESTING) {
			mark_parents_uninteresting(commit);
			revs->limited = 1;
		}
		if (revs->show_source && !commit->util)
			commit->util = (void *) name;
		return commit;
	}

	/*
	 * Tree object? Either mark it uninteresting, or add it
	 * to the list of objects to look at later..
	 */
	if (object->type == OBJ_TREE) {
		struct tree *tree = (struct tree *)object;
		if (!revs->tree_objects)
			return NULL;
		if (flags & UNINTERESTING) {
			mark_tree_contents_uninteresting(tree);
			return NULL;
		}
		add_pending_object(revs, object, "");
		return NULL;
	}

	/*
	 * Blob object? You know the drill by now..
	 */
	if (object->type == OBJ_BLOB) {
		if (!revs->blob_objects)
			return NULL;
		if (flags & UNINTERESTING)
			return NULL;
		add_pending_object(revs, object, "");
		return NULL;
	}
	die("%s is unknown object", name);
}