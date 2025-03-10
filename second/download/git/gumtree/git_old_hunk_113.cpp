	}

	if (gitmodules_modified)
		stage_updated_gitmodules();

	if (active_cache_changed) {
		if (write_cache(newfd, active_cache, active_nr) ||
		    commit_locked_index(&lock_file))
			die(_("Unable to write new index file"));
	}

	return 0;
}
