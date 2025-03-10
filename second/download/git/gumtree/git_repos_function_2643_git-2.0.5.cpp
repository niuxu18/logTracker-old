static const char *path_ok(char *directory)
{
	static char rpath[PATH_MAX];
	static char interp_path[PATH_MAX];
	const char *path;
	char *dir;

	dir = directory;

	if (daemon_avoid_alias(dir)) {
		logerror("'%s': aliased", dir);
		return NULL;
	}

	if (*dir == '~') {
		if (!user_path) {
			logerror("'%s': User-path not allowed", dir);
			return NULL;
		}
		if (*user_path) {
			/* Got either "~alice" or "~alice/foo";
			 * rewrite them to "~alice/%s" or
			 * "~alice/%s/foo".
			 */
			int namlen, restlen = strlen(dir);
			char *slash = strchr(dir, '/');
			if (!slash)
				slash = dir + restlen;
			namlen = slash - dir;
			restlen -= namlen;
			loginfo("userpath <%s>, request <%s>, namlen %d, restlen %d, slash <%s>", user_path, dir, namlen, restlen, slash);
			snprintf(rpath, PATH_MAX, "%.*s/%s%.*s",
				 namlen, dir, user_path, restlen, slash);
			dir = rpath;
		}
	}
	else if (interpolated_path && saw_extended_args) {
		struct strbuf expanded_path = STRBUF_INIT;
		struct strbuf_expand_dict_entry dict[6];

		dict[0].placeholder = "H"; dict[0].value = hostname;
		dict[1].placeholder = "CH"; dict[1].value = canon_hostname;
		dict[2].placeholder = "IP"; dict[2].value = ip_address;
		dict[3].placeholder = "P"; dict[3].value = tcp_port;
		dict[4].placeholder = "D"; dict[4].value = directory;
		dict[5].placeholder = NULL; dict[5].value = NULL;
		if (*dir != '/') {
			/* Allow only absolute */
			logerror("'%s': Non-absolute path denied (interpolated-path active)", dir);
			return NULL;
		}

		strbuf_expand(&expanded_path, interpolated_path,
				strbuf_expand_dict_cb, &dict);
		strlcpy(interp_path, expanded_path.buf, PATH_MAX);
		strbuf_release(&expanded_path);
		loginfo("Interpolated dir '%s'", interp_path);

		dir = interp_path;
	}
	else if (base_path) {
		if (*dir != '/') {
			/* Allow only absolute */
			logerror("'%s': Non-absolute path denied (base-path active)", dir);
			return NULL;
		}
		snprintf(rpath, PATH_MAX, "%s%s", base_path, dir);
		dir = rpath;
	}

	path = enter_repo(dir, strict_paths);
	if (!path && base_path && base_path_relaxed) {
		/*
		 * if we fail and base_path_relaxed is enabled, try without
		 * prefixing the base path
		 */
		dir = directory;
		path = enter_repo(dir, strict_paths);
	}

	if (!path) {
		logerror("'%s' does not appear to be a git repository", dir);
		return NULL;
	}

	if ( ok_paths && *ok_paths ) {
		char **pp;
		int pathlen = strlen(path);

		/* The validation is done on the paths after enter_repo
		 * appends optional {.git,.git/.git} and friends, but
		 * it does not use getcwd().  So if your /pub is
		 * a symlink to /mnt/pub, you can whitelist /pub and
		 * do not have to say /mnt/pub.
		 * Do not say /pub/.
		 */
		for ( pp = ok_paths ; *pp ; pp++ ) {
			int len = strlen(*pp);
			if (len <= pathlen &&
			    !memcmp(*pp, path, len) &&
			    (path[len] == '\0' ||
			     (!strict_paths && path[len] == '/')))
				return path;
		}
	}
	else {
		/* be backwards compatible */
		if (!strict_paths)
			return path;
	}

	logerror("'%s': not in whitelist", path);
	return NULL;		/* Fallthrough. Deny by default */
}