{
	int want_type;

	/* Check if ACL is NFSv4 */
	if ((acl->acl_types & ARCHIVE_ENTRY_ACL_TYPE_NFS4) != 0) {
		/* NFSv4 should never mix with POSIX.1e */
		if ((acl->acl_types & ARCHIVE_ENTRY_ACL_TYPE_POSIX1E) != 0)
			return (0);
		else
			return (ARCHIVE_ENTRY_ACL_TYPE_NFS4);
	}

	/* Now deal with POSIX.1e ACLs */

	want_type = 0;
	if ((flags & ARCHIVE_ENTRY_ACL_TYPE_ACCESS) != 0)
		want_type |= ARCHIVE_ENTRY_ACL_TYPE_ACCESS;
	if ((flags & ARCHIVE_ENTRY_ACL_TYPE_DEFAULT) != 0)
		want_type |= ARCHIVE_ENTRY_ACL_TYPE_DEFAULT;

	/* By default we want both access and default ACLs */
	if (want_type == 0)
		return (ARCHIVE_ENTRY_ACL_TYPE_POSIX1E);

	return (want_type);
}