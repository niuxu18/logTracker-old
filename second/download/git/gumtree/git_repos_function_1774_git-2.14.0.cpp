static void end_packfile(void)
{
	static int running;

	if (running || !pack_data)
		return;

	running = 1;
	clear_delta_base_cache();
	if (object_count) {
		struct packed_git *new_p;
		struct object_id cur_pack_oid;
		char *idx_name;
		int i;
		struct branch *b;
		struct tag *t;

		close_pack_windows(pack_data);
		sha1close(pack_file, cur_pack_oid.hash, 0);
		fixup_pack_header_footer(pack_data->pack_fd, pack_data->sha1,
				    pack_data->pack_name, object_count,
				    cur_pack_oid.hash, pack_size);

		if (object_count <= unpack_limit) {
			if (!loosen_small_pack(pack_data)) {
				invalidate_pack_id(pack_id);
				goto discard_pack;
			}
		}

		close(pack_data->pack_fd);
		idx_name = keep_pack(create_index());

		/* Register the packfile with core git's machinery. */
		new_p = add_packed_git(idx_name, strlen(idx_name), 1);
		if (!new_p)
			die("core git rejected index %s", idx_name);
		all_packs[pack_id] = new_p;
		install_packed_git(new_p);
		free(idx_name);

		/* Print the boundary */
		if (pack_edges) {
			fprintf(pack_edges, "%s:", new_p->pack_name);
			for (i = 0; i < branch_table_sz; i++) {
				for (b = branch_table[i]; b; b = b->table_next_branch) {
					if (b->pack_id == pack_id)
						fprintf(pack_edges, " %s",
							oid_to_hex(&b->oid));
				}
			}
			for (t = first_tag; t; t = t->next_tag) {
				if (t->pack_id == pack_id)
					fprintf(pack_edges, " %s",
						oid_to_hex(&t->oid));
			}
			fputc('\n', pack_edges);
			fflush(pack_edges);
		}

		pack_id++;
	}
	else {
discard_pack:
		close(pack_data->pack_fd);
		unlink_or_warn(pack_data->pack_name);
	}
	FREE_AND_NULL(pack_data);
	running = 0;

	/* We can't carry a delta across packfiles. */
	strbuf_release(&last_blob.data);
	last_blob.offset = 0;
	last_blob.depth = 0;
}