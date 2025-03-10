		o.branch1 = head_arg;
		o.branch2 = merge_remote_util(remoteheads->item)->name;

		for (j = common; j; j = j->next)
			commit_list_insert(j->item, &reversed);

		hold_locked_index(&lock, 1);
		clean = merge_recursive(&o, head,
				remoteheads->item, reversed, &result);
		if (active_cache_changed &&
		    write_locked_index(&the_index, &lock, COMMIT_LOCK))
			die (_("unable to write %s"), get_index_file());
		rollback_lock_file(&lock);
		return clean ? 0 : 1;
	} else {
		return try_merge_command(strategy, xopts_nr, xopts,
						common, head_arg, remoteheads);
	}
}
