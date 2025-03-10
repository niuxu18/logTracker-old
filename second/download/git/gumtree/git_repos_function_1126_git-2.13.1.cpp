static void handle_one_reflog_commit(struct object_id *oid, void *cb_data)
{
	struct all_refs_cb *cb = cb_data;
	if (!is_null_oid(oid)) {
		struct object *o = parse_object(oid->hash);
		if (o) {
			o->flags |= cb->all_flags;
			/* ??? CMDLINEFLAGS ??? */
			add_pending_object(cb->all_revs, o, "");
		}
		else if (!cb->warned_bad_reflog) {
			warning("reflog of '%s' references pruned commits",
				cb->name_for_errormsg);
			cb->warned_bad_reflog = 1;
		}
	}
}