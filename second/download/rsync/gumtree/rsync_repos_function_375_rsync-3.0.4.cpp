int set_file_attrs(const char *fname, struct file_struct *file, stat_x *sxp,
		   const char *fnamecmp, int flags)
{
	int updated = 0;
	stat_x sx2;
	int change_uid, change_gid;
	mode_t new_mode = file->mode;
	int inherit;

	if (!sxp) {
		if (dry_run)
			return 1;
		if (link_stat(fname, &sx2.st, 0) < 0) {
			rsyserr(FERROR_XFER, errno, "stat %s failed",
				full_fname(fname));
			return 0;
		}
#ifdef SUPPORT_ACLS
		sx2.acc_acl = sx2.def_acl = NULL;
#endif
#ifdef SUPPORT_XATTRS
		sx2.xattr = NULL;
#endif
		sxp = &sx2;
		inherit = !preserve_perms;
	} else
		inherit = !preserve_perms && file->flags & FLAG_DIR_CREATED;

	if (inherit && S_ISDIR(new_mode) && sxp->st.st_mode & S_ISGID) {
		/* We just created this directory and its setgid
		 * bit is on, so make sure it stays on. */
		new_mode |= S_ISGID;
	}

	if (daemon_chmod_modes && !S_ISLNK(new_mode))
		new_mode = tweak_mode(new_mode, daemon_chmod_modes);

#ifdef SUPPORT_ACLS
	if (preserve_acls && !S_ISLNK(file->mode) && !ACL_READY(*sxp))
		get_acl(fname, sxp);
#endif

#ifdef SUPPORT_XATTRS
	if (am_root < 0)
		set_stat_xattr(fname, file, new_mode);
	if (preserve_xattrs && fnamecmp)
		set_xattr(fname, file, fnamecmp, sxp);
#endif

	if (!preserve_times || (S_ISDIR(sxp->st.st_mode) && preserve_times == 1))
		flags |= ATTRS_SKIP_MTIME;
	if (!(flags & ATTRS_SKIP_MTIME)
	    && cmp_time(sxp->st.st_mtime, file->modtime) != 0) {
		int ret = set_modtime(fname, file->modtime, sxp->st.st_mode);
		if (ret < 0) {
			rsyserr(FERROR_XFER, errno, "failed to set times on %s",
				full_fname(fname));
			goto cleanup;
		}
		if (ret == 0) /* ret == 1 if symlink could not be set */
			updated = 1;
		else
			file->flags |= FLAG_TIME_FAILED;
	}

	change_uid = am_root && uid_ndx && sxp->st.st_uid != (uid_t)F_OWNER(file);
	change_gid = gid_ndx && !(file->flags & FLAG_SKIP_GROUP)
		  && sxp->st.st_gid != (gid_t)F_GROUP(file);
#if !defined HAVE_LCHOWN && !defined CHOWN_MODIFIES_SYMLINK
	if (S_ISLNK(sxp->st.st_mode)) {
		;
	} else
#endif
	if (change_uid || change_gid) {
		if (verbose > 2) {
			if (change_uid) {
				rprintf(FINFO,
					"set uid of %s from %u to %u\n",
					fname, (unsigned)sxp->st.st_uid, F_OWNER(file));
			}
			if (change_gid) {
				rprintf(FINFO,
					"set gid of %s from %u to %u\n",
					fname, (unsigned)sxp->st.st_gid, F_GROUP(file));
			}
		}
		if (am_root >= 0) {
			if (do_lchown(fname,
			    change_uid ? (uid_t)F_OWNER(file) : sxp->st.st_uid,
			    change_gid ? (gid_t)F_GROUP(file) : sxp->st.st_gid) != 0) {
				/* We shouldn't have attempted to change uid
				 * or gid unless have the privilege. */
				rsyserr(FERROR_XFER, errno, "%s %s failed",
				    change_uid ? "chown" : "chgrp",
				    full_fname(fname));
				goto cleanup;
			}
			/* A lchown had been done, so we need to re-stat if
			 * the destination had the setuid or setgid bits set
			 * (due to the side effect of the chown call). */
			if (sxp->st.st_mode & (S_ISUID | S_ISGID)) {
				link_stat(fname, &sxp->st,
					  keep_dirlinks && S_ISDIR(sxp->st.st_mode));
			}
		}
		updated = 1;
	}

#ifdef SUPPORT_ACLS
	/* It's OK to call set_acl() now, even for a dir, as the generator
	 * will enable owner-writability using chmod, if necessary.
	 * 
	 * If set_acl() changes permission bits in the process of setting
	 * an access ACL, it changes sxp->st.st_mode so we know whether we
	 * need to chmod(). */
	if (preserve_acls && !S_ISLNK(new_mode) && set_acl(fname, file, sxp) == 0)
		updated = 1;
#endif

#ifdef HAVE_CHMOD
	if (!BITS_EQUAL(sxp->st.st_mode, new_mode, CHMOD_BITS)) {
		int ret = am_root < 0 ? 0 : do_chmod(fname, new_mode);
		if (ret < 0) {
			rsyserr(FERROR_XFER, errno,
				"failed to set permissions on %s",
				full_fname(fname));
			goto cleanup;
		}
		if (ret == 0) /* ret == 1 if symlink could not be set */
			updated = 1;
	}
#endif

	if (verbose > 1 && flags & ATTRS_REPORT) {
		if (updated)
			rprintf(FCLIENT, "%s\n", fname);
		else
			rprintf(FCLIENT, "%s is uptodate\n", fname);
	}
  cleanup:
	if (sxp == &sx2) {
#ifdef SUPPORT_ACLS
		if (preserve_acls)
			free_acl(&sx2);
#endif
#ifdef SUPPORT_XATTRS
		if (preserve_xattrs)
			free_xattr(&sx2);
#endif
	}
	return updated;
}