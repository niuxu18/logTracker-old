int recv_files(int f_in, char *local_name)
{
	int next_gen_ndx = -1;
	int fd1,fd2;
	STRUCT_STAT st;
	int iflags, xlen;
	char *fname, fbuf[MAXPATHLEN];
	char xname[MAXPATHLEN];
	char fnametmp[MAXPATHLEN];
	char *fnamecmp, *partialptr;
	char fnamecmpbuf[MAXPATHLEN];
	uchar fnamecmp_type;
	struct file_struct *file;
	struct stats initial_stats;
	int itemizing = am_server ? logfile_format_has_i : stdout_format_has_i;
	enum logcode log_code = log_before_transfer ? FLOG : FINFO;
	int max_phase = protocol_version >= 29 ? 2 : 1;
	int dflt_perms = (ACCESSPERMS & ~orig_umask);
#ifdef SUPPORT_ACLS
	const char *parent_dirname = "";
#endif
	int ndx, recv_ok;

	if (verbose > 2)
		rprintf(FINFO, "recv_files(%d) starting\n", cur_flist->used);

	if (delay_updates)
		delayed_bits = bitbag_create(cur_flist->used + 1);

	while (1) {
		cleanup_disable();

		/* This call also sets cur_flist. */
		ndx = read_ndx_and_attrs(f_in, &iflags, &fnamecmp_type,
					 xname, &xlen);
		if (ndx == NDX_DONE) {
			if (inc_recurse && first_flist) {
				flist_free(first_flist);
				if (first_flist)
					continue;
			}
			if (read_batch && cur_flist) {
				int high = inc_recurse
				    ? first_flist->prev->used + first_flist->prev->ndx_start
				    : cur_flist->used;
				get_next_gen_ndx(batch_gen_fd, next_gen_ndx, high);
				next_gen_ndx = -1;
			}
			if (++phase > max_phase)
				break;
			if (verbose > 2)
				rprintf(FINFO, "recv_files phase=%d\n", phase);
			if (phase == 2 && delay_updates)
				handle_delayed_updates(local_name);
			send_msg(MSG_DONE, "", 0, 0);
			continue;
		}

		if (ndx - cur_flist->ndx_start >= 0)
			file = cur_flist->files[ndx - cur_flist->ndx_start];
		else
			file = dir_flist->files[cur_flist->parent_ndx];
		fname = local_name ? local_name : f_name(file, fbuf);

		if (verbose > 2)
			rprintf(FINFO, "recv_files(%s)\n", fname);

#ifdef SUPPORT_XATTRS
		if (iflags & ITEM_REPORT_XATTR && !dry_run)
			recv_xattr_request(file, f_in);
#endif

		if (!(iflags & ITEM_TRANSFER)) {
			maybe_log_item(file, iflags, itemizing, xname);
#ifdef SUPPORT_XATTRS
			if (preserve_xattrs && iflags & ITEM_REPORT_XATTR && !dry_run)
				set_file_attrs(fname, file, NULL, fname, 0);
#endif
			continue;
		}
		if (phase == 2) {
			rprintf(FERROR,
				"got transfer request in phase 2 [%s]\n",
				who_am_i());
			exit_cleanup(RERR_PROTOCOL);
		}

		if (file->flags & FLAG_FILE_SENT) {
			if (csum_length == SHORT_SUM_LENGTH) {
				if (keep_partial && !partial_dir)
					make_backups = -make_backups; /* prevents double backup */
				if (append_mode)
					sparse_files = -sparse_files;
				append_mode = -append_mode;
				csum_length = SUM_LENGTH;
				redoing = 1;
			}
		} else {
			if (csum_length != SHORT_SUM_LENGTH) {
				if (keep_partial && !partial_dir)
					make_backups = -make_backups;
				if (append_mode)
					sparse_files = -sparse_files;
				append_mode = -append_mode;
				csum_length = SHORT_SUM_LENGTH;
				redoing = 0;
			}
		}

		if (!am_server && do_progress)
			set_current_file_index(file, ndx);
		stats.num_transferred_files++;
		stats.total_transferred_size += F_LENGTH(file);

		cleanup_got_literal = 0;

		if (daemon_filter_list.head
		    && check_filter(&daemon_filter_list, FLOG, fname, 0) < 0) {
			rprintf(FERROR, "attempt to hack rsync failed.\n");
			exit_cleanup(RERR_PROTOCOL);
		}

		if (!do_xfers) { /* log the transfer */
			log_item(FCLIENT, file, &stats, iflags, NULL);
			if (read_batch)
				discard_receive_data(f_in, F_LENGTH(file));
			continue;
		}
		if (write_batch < 0) {
			log_item(FCLIENT, file, &stats, iflags, NULL);
			if (!am_server)
				discard_receive_data(f_in, F_LENGTH(file));
			continue;
		}

		if (read_batch) {
			next_gen_ndx = get_next_gen_ndx(batch_gen_fd, next_gen_ndx, ndx);
			if (ndx < next_gen_ndx) {
				rprintf(FINFO,
					"(Skipping batched update for \"%s\")\n",
					fname);
				discard_receive_data(f_in, F_LENGTH(file));
				if (inc_recurse)
					send_msg_int(MSG_NO_SEND, ndx);
				continue;
			}
			next_gen_ndx = -1;
		}

		partialptr = partial_dir ? partial_dir_fname(fname) : fname;

		if (protocol_version >= 29) {
			switch (fnamecmp_type) {
			case FNAMECMP_FNAME:
				fnamecmp = fname;
				break;
			case FNAMECMP_PARTIAL_DIR:
				fnamecmp = partialptr;
				break;
			case FNAMECMP_BACKUP:
				fnamecmp = get_backup_name(fname);
				break;
			case FNAMECMP_FUZZY:
				if (file->dirname) {
					pathjoin(fnamecmpbuf, MAXPATHLEN,
						 file->dirname, xname);
					fnamecmp = fnamecmpbuf;
				} else
					fnamecmp = xname;
				break;
			default:
				if (fnamecmp_type >= basis_dir_cnt) {
					rprintf(FERROR,
						"invalid basis_dir index: %d.\n",
						fnamecmp_type);
					exit_cleanup(RERR_PROTOCOL);
				}
				pathjoin(fnamecmpbuf, sizeof fnamecmpbuf,
					 basis_dir[fnamecmp_type], fname);
				fnamecmp = fnamecmpbuf;
				break;
			}
			if (!fnamecmp || (daemon_filter_list.head
			  && check_filter(&daemon_filter_list, FLOG, fname, 0) < 0)) {
				fnamecmp = fname;
				fnamecmp_type = FNAMECMP_FNAME;
			}
		} else {
			/* Reminder: --inplace && --partial-dir are never
			 * enabled at the same time. */
			if (inplace && make_backups > 0) {
				if (!(fnamecmp = get_backup_name(fname)))
					fnamecmp = fname;
				else
					fnamecmp_type = FNAMECMP_BACKUP;
			} else if (partial_dir && partialptr)
				fnamecmp = partialptr;
			else
				fnamecmp = fname;
		}

		initial_stats = stats;

		/* open the file */
		fd1 = do_open(fnamecmp, O_RDONLY, 0);

		if (fd1 == -1 && protocol_version < 29) {
			if (fnamecmp != fname) {
				fnamecmp = fname;
				fd1 = do_open(fnamecmp, O_RDONLY, 0);
			}

			if (fd1 == -1 && basis_dir[0]) {
				/* pre-29 allowed only one alternate basis */
				pathjoin(fnamecmpbuf, sizeof fnamecmpbuf,
					 basis_dir[0], fname);
				fnamecmp = fnamecmpbuf;
				fd1 = do_open(fnamecmp, O_RDONLY, 0);
			}
		}

		updating_basis_or_equiv = inplace
		    && (fnamecmp == fname || fnamecmp_type == FNAMECMP_BACKUP);

		if (fd1 == -1) {
			st.st_mode = 0;
			st.st_size = 0;
		} else if (do_fstat(fd1,&st) != 0) {
			rsyserr(FERROR_XFER, errno, "fstat %s failed",
				full_fname(fnamecmp));
			discard_receive_data(f_in, F_LENGTH(file));
			close(fd1);
			if (inc_recurse)
				send_msg_int(MSG_NO_SEND, ndx);
			continue;
		}

		if (fd1 != -1 && S_ISDIR(st.st_mode) && fnamecmp == fname) {
			/* this special handling for directories
			 * wouldn't be necessary if robust_rename()
			 * and the underlying robust_unlink could cope
			 * with directories
			 */
			rprintf(FERROR_XFER, "recv_files: %s is a directory\n",
				full_fname(fnamecmp));
			discard_receive_data(f_in, F_LENGTH(file));
			close(fd1);
			if (inc_recurse)
				send_msg_int(MSG_NO_SEND, ndx);
			continue;
		}

		if (fd1 != -1 && !S_ISREG(st.st_mode)) {
			close(fd1);
			fd1 = -1;
		}

		/* If we're not preserving permissions, change the file-list's
		 * mode based on the local permissions and some heuristics. */
		if (!preserve_perms) {
			int exists = fd1 != -1;
#ifdef SUPPORT_ACLS
			const char *dn = file->dirname ? file->dirname : ".";
			if (parent_dirname != dn
			 && strcmp(parent_dirname, dn) != 0) {
				dflt_perms = default_perms_for_dir(dn);
				parent_dirname = dn;
			}
#endif
			file->mode = dest_mode(file->mode, st.st_mode,
					       dflt_perms, exists);
		}

		/* We now check to see if we are writing the file "inplace" */
		if (inplace)  {
			fd2 = do_open(fname, O_WRONLY|O_CREAT, 0600);
			if (fd2 == -1) {
				rsyserr(FERROR_XFER, errno, "open %s failed",
					full_fname(fname));
			}
		} else {
			fd2 = open_tmpfile(fnametmp, fname, file);
			if (fd2 != -1)
				cleanup_set(fnametmp, partialptr, file, fd1, fd2);
		}

		if (fd2 == -1) {
			discard_receive_data(f_in, F_LENGTH(file));
			if (fd1 != -1)
				close(fd1);
			if (inc_recurse)
				send_msg_int(MSG_NO_SEND, ndx);
			continue;
		}

		/* log the transfer */
		if (log_before_transfer)
			log_item(FCLIENT, file, &initial_stats, iflags, NULL);
		else if (!am_server && verbose && do_progress)
			rprintf(FINFO, "%s\n", fname);

		/* recv file data */
		recv_ok = receive_data(f_in, fnamecmp, fd1, st.st_size,
				       fname, fd2, F_LENGTH(file));

		log_item(log_code, file, &initial_stats, iflags, NULL);

		if (fd1 != -1)
			close(fd1);
		if (close(fd2) < 0) {
			rsyserr(FERROR, errno, "close failed on %s",
				full_fname(fnametmp));
			exit_cleanup(RERR_FILEIO);
		}

		if ((recv_ok && (!delay_updates || !partialptr)) || inplace) {
			if (partialptr == fname)
				partialptr = NULL;
			if (!finish_transfer(fname, fnametmp, fnamecmp,
					     partialptr, file, recv_ok, 1))
				recv_ok = -1;
			else if (fnamecmp == partialptr) {
				do_unlink(partialptr);
				handle_partial_dir(partialptr, PDIR_DELETE);
			}
		} else if (keep_partial && partialptr) {
			if (!handle_partial_dir(partialptr, PDIR_CREATE)) {
				rprintf(FERROR,
				    "Unable to create partial-dir for %s -- discarding %s.\n",
				    local_name ? local_name : f_name(file, NULL),
				    recv_ok ? "completed file" : "partial file");
				do_unlink(fnametmp);
				recv_ok = -1;
			} else if (!finish_transfer(partialptr, fnametmp, fnamecmp, NULL,
						    file, recv_ok, !partial_dir))
				recv_ok = -1;
			else if (delay_updates && recv_ok) {
				bitbag_set_bit(delayed_bits, ndx);
				recv_ok = 2;
			} else
				partialptr = NULL;
		} else
			do_unlink(fnametmp);

		cleanup_disable();

		switch (recv_ok) {
		case 2:
			break;
		case 1:
			if (remove_source_files || inc_recurse
			 || (preserve_hard_links && F_IS_HLINKED(file)))
				send_msg_int(MSG_SUCCESS, ndx);
			break;
		case 0: {
			enum logcode msgtype = redoing ? FERROR_XFER : FWARNING;
			if (msgtype == FERROR_XFER || verbose) {
				char *errstr, *redostr, *keptstr;
				if (!(keep_partial && partialptr) && !inplace)
					keptstr = "discarded";
				else if (partial_dir)
					keptstr = "put into partial-dir";
				else
					keptstr = "retained";
				if (msgtype == FERROR_XFER) {
					errstr = "ERROR";
					redostr = "";
				} else {
					errstr = "WARNING";
					redostr = read_batch ? " (may try again)"
							     : " (will try again)";
				}
				rprintf(msgtype,
					"%s: %s failed verification -- update %s%s.\n",
					errstr, local_name ? f_name(file, NULL) : fname,
					keptstr, redostr);
			}
			if (!redoing) {
				send_msg_int(MSG_REDO, ndx);
				file->flags |= FLAG_FILE_SENT;
			} else if (inc_recurse)
				send_msg_int(MSG_NO_SEND, ndx);
			break;
		    }
		case -1:
			if (inc_recurse)
				send_msg_int(MSG_NO_SEND, ndx);
			break;
		}
	}
	if (make_backups < 0)
		make_backups = -make_backups;

	if (phase == 2 && delay_updates) /* for protocol_version < 29 */
		handle_delayed_updates(local_name);

	if (verbose > 2)
		rprintf(FINFO,"recv_files finished\n");

	return 0;
}