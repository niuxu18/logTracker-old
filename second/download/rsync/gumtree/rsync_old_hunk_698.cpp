		io_printf(f_out, "@ERROR: auth failed on module %s\n", name);
		return -1;
	}

	module_id = i;

	am_root = (getuid() == 0);

	if (am_root) {
		p = lp_uid(i);
		if (!name_to_uid(p, &uid)) {
			if (!isdigit(* (unsigned char *) p)) {
				rprintf(FERROR,"Invalid uid %s\n", p);
