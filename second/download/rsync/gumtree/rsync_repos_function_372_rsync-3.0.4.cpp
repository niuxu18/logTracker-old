int read_ndx_and_attrs(int f_in, int *iflag_ptr, uchar *type_ptr,
		       char *buf, int *len_ptr)
{
	int len, iflags = 0;
	struct file_list *flist;
	uchar fnamecmp_type = FNAMECMP_FNAME;
	int ndx, save_verbose = verbose;

  read_loop:
	while (1) {
		ndx = read_ndx(f_in);

		if (ndx >= 0)
			break;
		if (ndx == NDX_DONE)
			return ndx;
		if (!inc_recurse || am_sender) {
			int last;
			if (first_flist)
				last = first_flist->prev->ndx_start + first_flist->prev->used - 1;
			else
				last = -1;
			rprintf(FERROR,
				"Invalid file index: %d (%d - %d) [%s]\n",
				ndx, NDX_DONE, last, who_am_i());
			exit_cleanup(RERR_PROTOCOL);
		}
		if (ndx == NDX_FLIST_EOF) {
			flist_eof = 1;
			send_msg(MSG_FLIST_EOF, "", 0, 0);
			continue;
		}
		ndx = NDX_FLIST_OFFSET - ndx;
		if (ndx < 0 || ndx >= dir_flist->used) {
			ndx = NDX_FLIST_OFFSET - ndx;
			rprintf(FERROR,
				"Invalid dir index: %d (%d - %d) [%s]\n",
				ndx, NDX_FLIST_OFFSET,
				NDX_FLIST_OFFSET - dir_flist->used + 1,
				who_am_i());
			exit_cleanup(RERR_PROTOCOL);
		}

		/* Send everything read from f_in to msg_fd_out. */
		if (verbose > 3) {
			rprintf(FINFO, "[%s] receiving flist for dir %d\n",
				who_am_i(), ndx);
		}
		verbose = 0;
		send_msg_int(MSG_FLIST, ndx);
		start_flist_forward(f_in);
		flist = recv_file_list(f_in);
		flist->parent_ndx = ndx;
		stop_flist_forward();
		verbose = save_verbose;
	}

	iflags = protocol_version >= 29 ? read_shortint(f_in)
		   : ITEM_TRANSFER | ITEM_MISSING_DATA;

	/* Honor the old-style keep-alive indicator. */
	if (protocol_version < 30
	 && ndx == cur_flist->used && iflags == ITEM_IS_NEW) {
		if (am_sender)
			maybe_send_keepalive();
		goto read_loop;
	}

	cur_flist = flist_for_ndx(ndx, "read_ndx_and_attrs");

	if (iflags & ITEM_BASIS_TYPE_FOLLOWS)
		fnamecmp_type = read_byte(f_in);
	*type_ptr = fnamecmp_type;

	if (iflags & ITEM_XNAME_FOLLOWS) {
		if ((len = read_vstring(f_in, buf, MAXPATHLEN)) < 0)
			exit_cleanup(RERR_PROTOCOL);
	} else {
		*buf = '\0';
		len = -1;
	}
	*len_ptr = len;

	if (iflags & ITEM_TRANSFER) {
		int i = ndx - cur_flist->ndx_start;
		if (i < 0 || !S_ISREG(cur_flist->files[i]->mode)) {
			rprintf(FERROR,
				"received request to transfer non-regular file: %d [%s]\n",
				ndx, who_am_i());
			exit_cleanup(RERR_PROTOCOL);
		}
	}

	*iflag_ptr = iflags;
	return ndx;
}