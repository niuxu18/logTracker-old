	if (preserve_uid) {
		/* read the uid list */
		list = uidlist;
		id = read_int(f);
		while (id != 0) {
			int len = read_byte(f);
			name = new_array(char, len+1);
			if (!name) out_of_memory("recv_uid_list");
			read_sbuf(f, name, len);
			if (!list) {
				uidlist = add_list(id, name);
				list = uidlist;
			} else {
