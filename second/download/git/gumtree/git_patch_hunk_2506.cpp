 		refresh_cache_or_die(refresh_flags);
 
 		if (write_locked_index(&the_index, &index_lock, CLOSE_LOCK))
 			die(_("unable to create temporary index"));
 
 		old_index_env = getenv(INDEX_ENVIRONMENT);
-		setenv(INDEX_ENVIRONMENT, index_lock.filename.buf, 1);
+		setenv(INDEX_ENVIRONMENT, get_lock_file_path(&index_lock), 1);
 
 		if (interactive_add(argc, argv, prefix, patch_interactive) != 0)
 			die(_("interactive add failed"));
 
 		if (old_index_env && *old_index_env)
 			setenv(INDEX_ENVIRONMENT, old_index_env, 1);
 		else
 			unsetenv(INDEX_ENVIRONMENT);
 
 		discard_cache();
-		read_cache_from(index_lock.filename.buf);
+		read_cache_from(get_lock_file_path(&index_lock));
 		if (update_main_cache_tree(WRITE_TREE_SILENT) == 0) {
 			if (reopen_lock_file(&index_lock) < 0)
 				die(_("unable to write index file"));
 			if (write_locked_index(&the_index, &index_lock, CLOSE_LOCK))
 				die(_("unable to update temporary index"));
 		} else
 			warning(_("Failed to update main cache tree"));
 
 		commit_style = COMMIT_NORMAL;
-		return index_lock.filename.buf;
+		return get_lock_file_path(&index_lock);
 	}
 
 	/*
 	 * Non partial, non as-is commit.
 	 *
 	 * (1) get the real index;
