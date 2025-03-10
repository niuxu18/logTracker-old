	while (tree_entry(&desc, &entry)) {
		int result;

		if (S_ISGITLINK(entry.mode))
			continue;
		if (S_ISDIR(entry.mode))
			result = options->walk(&lookup_tree(entry.oid->hash)->object, OBJ_TREE, data, options);
		else if (S_ISREG(entry.mode) || S_ISLNK(entry.mode))
			result = options->walk(&lookup_blob(entry.oid->hash)->object, OBJ_BLOB, data, options);
		else {
			result = error("in tree %s: entry %s has bad mode %.6o",
					oid_to_hex(&tree->object.oid), entry.path, entry.mode);
		}
		if (result < 0)
			return result;
