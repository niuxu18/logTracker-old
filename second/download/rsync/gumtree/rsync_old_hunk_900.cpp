	if (uid == last_in)
		return last_out;

	last_in = uid;

	for (list = uidlist; list; list = list->next) {
		if (list->id == (int)uid)
			return last_out = (uid_t)list->id2;
	}

	return last_out = uid;
}

static gid_t match_gid(gid_t gid)
{
	static gid_t last_in = GID_NONE, last_out = GID_NONE;
	struct idlist *list;

	if (gid == GID_NONE)
		return GID_NONE;

	if (gid == last_in)
		return last_out;

	last_in = gid;

	for (list = gidlist; list; list = list->next) {
		if (list->id == (int)gid)
			return last_out = (gid_t)list->id2;
	}

	list = recv_add_gid(gid, NULL);
	return last_out = list->id2;
}

/* Add a uid to the list of uids.  Only called on sending side. */
void add_uid(uid_t uid)
{
	struct idlist *list;

	if (uid == 0)	/* don't map root */
		return;

	for (list = uidlist; list; list = list->next) {
		if (list->id == (int)uid)
			return;
	}

	add_to_list(&uidlist, (int)uid, uid_to_name(uid), 0);
}

/* Add a gid to the list of gids.  Only called on sending side. */
void add_gid(gid_t gid)
{
	struct idlist *list;

	if (gid == 0)	/* don't map root */
		return;

	for (list = gidlist; list; list = list->next) {
		if (list->id == (int)gid)
			return;
	}

	add_to_list(&gidlist, (int)gid, gid_to_name(gid), 0);
}


/* send a complete uid/gid mapping to the peer */
void send_uid_list(int f)
{
	struct idlist *list;

	if (numeric_ids)
		return;

	if (preserve_uid) {
		int len;
		/* we send sequences of uid/byte-length/name */
		for (list = uidlist; list; list = list->next) {
			if (!list->name)
				continue;
			len = strlen(list->name);
			write_int(f, list->id);
			write_byte(f, len);
			write_buf(f, list->name, len);
		}

		/* terminate the uid list with a 0 uid. We explicitly exclude
		 * 0 from the list */
		write_int(f, 0);
	}

	if (preserve_gid) {
		int len;
		for (list = gidlist; list; list = list->next) {
			if (!list->name)
				continue;
			len = strlen(list->name);
			write_int(f, list->id);
			write_byte(f, len);
			write_buf(f, list->name, len);
		}
		write_int(f, 0);
	}
}

/* recv a complete uid/gid mapping from the peer and map the uid/gid
 * in the file list to local names */
void recv_uid_list(int f, struct file_list *flist)
{
	int id, i;
	char *name;

	if (preserve_uid && !numeric_ids) {
		/* read the uid list */
		while ((id = read_int(f)) != 0) {
			int len = read_byte(f);
			name = new_array(char, len+1);
			if (!name)
				out_of_memory("recv_uid_list");
			read_sbuf(f, name, len);
			recv_add_uid(id, name); /* node keeps name's memory */
		}
	}

	if (preserve_gid && !numeric_ids) {
		/* read the gid list */
		while ((id = read_int(f)) != 0) {
			int len = read_byte(f);
			name = new_array(char, len+1);
			if (!name)
				out_of_memory("recv_uid_list");
			read_sbuf(f, name, len);
			recv_add_gid(id, name); /* node keeps name's memory */
		}
	}

	/* Now convert all the uids/gids from sender values to our values. */
	if (am_root && preserve_uid && !numeric_ids) {
		for (i = 0; i < flist->count; i++)
			flist->files[i]->uid = match_uid(flist->files[i]->uid);
	}
	if (preserve_gid && (!am_root || !numeric_ids)) {
		for (i = 0; i < flist->count; i++)
			flist->files[i]->gid = match_gid(flist->files[i]->gid);
	}
}
