	io_printf(f_out, "@ERROR: chdir failed\n");
	return -1;
}

static int add_a_group(int f_out, const char *gname)
{
	gid_t gid, *gid_p;
	if (!group_to_gid(gname, &gid, True)) {
		rprintf(FLOG, "Invalid gid %s\n", gname);
		io_printf(f_out, "@ERROR: invalid gid %s\n", gname);
		return -1;
	}
	gid_p = EXPAND_ITEM_LIST(&gid_list, gid_t, -32);
	*gid_p = gid;
	return 0;
}

#ifdef HAVE_GETGROUPLIST
static int want_all_groups(int f_out, uid_t uid)
{
	const char *err;
	if ((err = getallgroups(uid, &gid_list)) != NULL) {
		rsyserr(FLOG, errno, "%s", err);
		io_printf(f_out, "@ERROR: %s\n", err);
		return -1;
	}
	return 0;
}
#elif defined HAVE_INITGROUPS
static struct passwd *want_all_groups(int f_out, uid_t uid)
{
	struct passwd *pw;
	gid_t *gid_p;
	if ((pw = getpwuid(uid)) == NULL) {
		rsyserr(FLOG, errno, "getpwuid failed");
		io_printf(f_out, "@ERROR: getpwuid failed\n");
		return NULL;
	}
	/* Start with the default group and initgroups() will add the rest. */
	gid_p = EXPAND_ITEM_LIST(&gid_list, gid_t, -32);
	*gid_p = pw->pw_gid;
	return pw;
}
#endif

static void set_env_str(const char *var, const char *str)
{
