static void write_merge_state(struct commit_list *remoteheads)
{
	struct commit_list *j;
	struct strbuf buf = STRBUF_INIT;

	for (j = remoteheads; j; j = j->next) {
		struct object_id *oid;
		struct commit *c = j->item;
		if (c->util && merge_remote_util(c)->obj) {
			oid = &merge_remote_util(c)->obj->oid;
		} else {
			oid = &c->object.oid;
		}
		strbuf_addf(&buf, "%s\n", oid_to_hex(oid));
	}
	write_file_buf(git_path_merge_head(), buf.buf, buf.len);
	strbuf_addch(&merge_msg, '\n');
	write_file_buf(git_path_merge_msg(), merge_msg.buf, merge_msg.len);

	strbuf_reset(&buf);
	if (fast_forward == FF_NO)
		strbuf_addf(&buf, "no-ff");
	write_file_buf(git_path_merge_mode(), buf.buf, buf.len);
}