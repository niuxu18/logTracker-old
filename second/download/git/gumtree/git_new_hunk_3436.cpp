		die("bad tree %s", oid_to_hex(&obj->oid));

	init_tree_desc(&desc, tree->buffer, tree->size);
	while (tree_entry(&desc, &entry)) {
		switch (object_type(entry.mode)) {
		case OBJ_TREE:
			mark_tree_uninteresting(lookup_tree(entry.oid->hash));
			break;
		case OBJ_BLOB:
			mark_blob_uninteresting(lookup_blob(entry.oid->hash));
			break;
		default:
			/* Subproject commit - not in this repository */
			break;
		}
	}
