static void fsck_handle_reflog_oid(const char *refname, struct object_id *oid,
	unsigned long timestamp)
{
	struct object *obj;

	if (!is_null_oid(oid)) {
		obj = lookup_object(oid->hash);
		if (obj && (obj->flags & HAS_OBJ)) {
			if (timestamp && name_objects)
				add_decoration(fsck_walk_options.object_names,
					obj,
					xstrfmt("%s@{%ld}", refname, timestamp));
			obj->used = 1;
			mark_object_reachable(obj);
		} else {
			error("%s: invalid reflog entry %s", refname, oid_to_hex(oid));
			errors_found |= ERROR_REACHABLE;
		}
	}
}