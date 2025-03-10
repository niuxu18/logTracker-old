static const char *unpack(int err_fd, struct shallow_info *si)
{
	struct pack_header hdr;
	const char *hdr_err;
	int status;
	char hdr_arg[38];
	struct child_process child = CHILD_PROCESS_INIT;
	int fsck_objects = (receive_fsck_objects >= 0
			    ? receive_fsck_objects
			    : transfer_fsck_objects >= 0
			    ? transfer_fsck_objects
			    : 0);

	hdr_err = parse_pack_header(&hdr);
	if (hdr_err) {
		if (err_fd > 0)
			close(err_fd);
		return hdr_err;
	}
	snprintf(hdr_arg, sizeof(hdr_arg),
			"--pack_header=%"PRIu32",%"PRIu32,
			ntohl(hdr.hdr_version), ntohl(hdr.hdr_entries));

	if (si->nr_ours || si->nr_theirs) {
		alt_shallow_file = setup_temporary_shallow(si->shallow);
		argv_array_push(&child.args, "--shallow-file");
		argv_array_push(&child.args, alt_shallow_file);
	}

	if (ntohl(hdr.hdr_entries) < unpack_limit) {
		argv_array_pushl(&child.args, "unpack-objects", hdr_arg, NULL);
		if (quiet)
			argv_array_push(&child.args, "-q");
		if (fsck_objects)
			argv_array_pushf(&child.args, "--strict%s",
				fsck_msg_types.buf);
		child.no_stdout = 1;
		child.err = err_fd;
		child.git_cmd = 1;
		status = run_command(&child);
		if (status)
			return "unpack-objects abnormal exit";
	} else {
		char hostname[256];

		argv_array_pushl(&child.args, "index-pack",
				 "--stdin", hdr_arg, NULL);

		if (gethostname(hostname, sizeof(hostname)))
			xsnprintf(hostname, sizeof(hostname), "localhost");
		argv_array_pushf(&child.args,
				 "--keep=receive-pack %"PRIuMAX" on %s",
				 (uintmax_t)getpid(),
				 hostname);

		if (fsck_objects)
			argv_array_pushf(&child.args, "--strict%s",
				fsck_msg_types.buf);
		if (!reject_thin)
			argv_array_push(&child.args, "--fix-thin");
		child.out = -1;
		child.err = err_fd;
		child.git_cmd = 1;
		status = start_command(&child);
		if (status)
			return "index-pack fork failed";
		pack_lockfile = index_pack_lockfile(child.out);
		close(child.out);
		status = finish_command(&child);
		if (status)
			return "index-pack abnormal exit";
		reprepare_packed_git();
	}
	return NULL;
}