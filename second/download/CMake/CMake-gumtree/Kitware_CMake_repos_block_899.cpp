{
		archive_set_error(&a->archive, errno,
		    "Failed to read ACL brand");
		return (ARCHIVE_WARN);
	}