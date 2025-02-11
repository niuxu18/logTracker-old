{
	cp->count++;
	if ((cp->count & 1023) == 0)
		display_progress(cp->progress, cp->count);
}

static int add_one_ref(const char *path, const unsigned char *sha1, int flag, void *cb_data)
{
	struct object *object = parse_object_or_die(sha1, path);
	struct rev_info *revs = (struct rev_info *)cb_data;

	add_pending_object(revs, object, "");

	return 0;
}

/*
 * The traversal will have already marked us as SEEN, so we
 * only need to handle any progress reporting here.
 */
static void mark_object(struct object *obj, const struct name_path *path,
			const char *name, void *data)
{
	update_progress(data);
}

static void mark_commit(struct commit *c, void *data)
{
	mark_object(&c->object, NULL, NULL, data);
}

struct recent_data {
	struct rev_info *revs;
	unsigned long timestamp;
};

static void add_recent_object(const unsigned char *sha1,
			      unsigned long mtime,
			      struct recent_data *data)
{
	struct object *obj;
	enum object_type type;

	if (mtime <= data->timestamp)
		return;

	/*
	 * We do not want to call parse_object here, because
	 * inflating blobs and trees could be very expensive.
	 * However, we do need to know the correct type for
	 * later processing, and the revision machinery expects
	 * commits and tags to have been parsed.
	 */
	type = sha1_object_info(sha1, NULL);
	if (type < 0)
		die("unable to get object info for %s", sha1_to_hex(sha1));

	switch (type) {
	case OBJ_TAG:
	case OBJ_COMMIT:
		obj = parse_object_or_die(sha1, NULL);
		break;
	case OBJ_TREE:
		obj = (struct object *)lookup_tree(sha1);
		break;
	case OBJ_BLOB:
		obj = (struct object *)lookup_blob(sha1);
		break;
	default:
		die("unknown object type for %s: %s",
		    sha1_to_hex(sha1), typename(type));
	}

	if (!obj)
		die("unable to lookup %s", sha1_to_hex(sha1));

	add_pending_object(data->revs, obj, "");
}

static int add_recent_loose(const unsigned char *sha1,
			    const char *path, void *data)
{
	struct stat st;
	struct object *obj = lookup_object(sha1);

	if (obj && obj->flags & SEEN)
		return 0;

	if (stat(path, &st) < 0) {
		/*
		 * It's OK if an object went away during our iteration; this
		 * could be due to a simultaneous repack. But anything else
		 * we should abort, since we might then fail to mark objects
		 * which should not be pruned.
		 */
		if (errno == ENOENT)
			return 0;
		return error("unable to stat %s: %s",
			     sha1_to_hex(sha1), strerror(errno));
	}

	add_recent_object(sha1, st.st_mtime, data);
	return 0;
}

static int add_recent_packed(const unsigned char *sha1,
			     struct packed_git *p, uint32_t pos,
			     void *data)
{
	struct object *obj = lookup_object(sha1);

	if (obj && obj->flags & SEEN)
		return 0;
	add_recent_object(sha1, p->mtime, data);
	return 0;
}

int add_unseen_recent_objects_to_traversal(struct rev_info *revs,
					   unsigned long timestamp)
{
	struct recent_data data;
	int r;

	data.revs = revs;
	data.timestamp = timestamp;

	r = for_each_loose_object(add_recent_loose, &data);
	if (r)
		return r;
	return for_each_packed_object(add_recent_packed, &data);
}

void mark_reachable_objects(struct rev_info *revs, int mark_reflog,
			    unsigned long mark_recent,
			    struct progress *progress)
{
	struct connectivity_progress cp;

	/*
	 * Set up revision parsing, and mark us as being interested
