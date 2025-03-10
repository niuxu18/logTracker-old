void check_for_finished_files(int itemizing, enum logcode code, int check_redo)
{
	struct file_struct *file;
	struct file_list *flist;
	char fbuf[MAXPATHLEN];
	int ndx;

	while (1) {
#ifdef SUPPORT_HARD_LINKS
		if (preserve_hard_links && (ndx = get_hlink_num()) != -1) {
			flist = flist_for_ndx(ndx, "check_for_finished_files.1");
			file = flist->files[ndx - flist->ndx_start];
			assert(file->flags & FLAG_HLINKED);
			finish_hard_link(file, f_name(file, fbuf), ndx, NULL, itemizing, code, -1);
			flist->in_progress--;
			continue;
		}
#endif

		if (check_redo && (ndx = get_redo_num()) != -1) {
			csum_length = SUM_LENGTH;
			max_size = -max_size;
			min_size = -min_size;
			ignore_existing = -ignore_existing;
			ignore_non_existing = -ignore_non_existing;
			update_only = -update_only;
			always_checksum = -always_checksum;
			size_only = -size_only;
			append_mode = -append_mode;
			make_backups = -make_backups; /* avoid dup backup w/inplace */
			ignore_times++;

			flist = cur_flist;
			cur_flist = flist_for_ndx(ndx, "check_for_finished_files.2");

			file = cur_flist->files[ndx - cur_flist->ndx_start];
			if (solo_file)
				strlcpy(fbuf, solo_file, sizeof fbuf);
			else
				f_name(file, fbuf);
			recv_generator(fbuf, file, ndx, itemizing, code, sock_f_out);
			cur_flist->to_redo--;

			cur_flist = flist;

			csum_length = SHORT_SUM_LENGTH;
			max_size = -max_size;
			min_size = -min_size;
			ignore_existing = -ignore_existing;
			ignore_non_existing = -ignore_non_existing;
			update_only = -update_only;
			always_checksum = -always_checksum;
			size_only = -size_only;
			append_mode = -append_mode;
			make_backups = -make_backups;
			ignore_times--;
			continue;
		}

		if (cur_flist == first_flist)
			break;

		/* We only get here if inc_recurse is enabled. */
		if (first_flist->in_progress || first_flist->to_redo)
			break;

		if (!read_batch) {
			write_ndx(sock_f_out, NDX_DONE);
			maybe_flush_socket(1);
		}

		if (delete_during == 2 || !dir_tweaking) {
			/* Skip directory touch-up. */
		} else if (first_flist->parent_ndx >= 0)
			touch_up_dirs(dir_flist, first_flist->parent_ndx);

		flist_free(first_flist); /* updates first_flist */
	}
}