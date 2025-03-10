		}
		free(seen);
	}

	plug_bulk_checkin();

	exit_status |= add_files_to_cache(prefix, &pathspec, flags);

	if (add_new_files)
		exit_status |= add_files(&dir, flags);

	if (chmod_arg && pathspec.nr)
		chmod_pathspec(&pathspec, chmod_arg[0]);
	unplug_bulk_checkin();

finish:
	if (active_cache_changed) {
		if (write_locked_index(&the_index, &lock_file, COMMIT_LOCK))
			die(_("Unable to write new index file"));
