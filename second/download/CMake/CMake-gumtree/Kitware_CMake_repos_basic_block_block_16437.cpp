do {
		if (np->virtual &&
		    !archive_entry_mtime_is_set(np->file->entry)) {
			/* Set properly times to virtual directory */
			archive_entry_set_mtime(np->file->entry,
			    iso9660->birth_time, 0);
			archive_entry_set_atime(np->file->entry,
			    iso9660->birth_time, 0);
			archive_entry_set_ctime(np->file->entry,
			    iso9660->birth_time, 0);
		}
		if (np->children.first != NULL) {
			if (vdd->vdd_type != VDD_JOLIET &&
			    !iso9660->opt.rr && depth + 1 >= vdd->max_depth) {
				if (np->children.cnt > 0)
					iso9660->directories_too_deep = np;
			} else {
				/* Generate Identifier */
				r = genid(a, np, &idr);
				if (r < 0)
					goto exit_traverse_tree;
				r = isoent_make_sorted_files(a, np, &idr);
				if (r < 0)
					goto exit_traverse_tree;

				if (np->subdirs.first != NULL &&
				    depth + 1 < vdd->max_depth) {
					/* Enter to sub directories. */
					np = np->subdirs.first;
					depth++;
					continue;
				}
			}
		}
		while (np != np->parent) {
			if (np->drnext == NULL) {
				/* Return to the parent directory. */
				np = np->parent;
				depth--;
			} else {
				np = np->drnext;
				break;
			}
		}
	} while (np != np->parent);