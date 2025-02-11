static int
isoent_make_sorted_files(struct archive_write *a, struct isoent *isoent,
    struct idr *idr)
{
	struct archive_rb_node *rn;
	struct isoent **children;

	children = malloc(isoent->children.cnt * sizeof(struct isoent *));
	if (children == NULL) {
		archive_set_error(&a->archive, ENOMEM,
		    "Can't allocate memory");
		return (ARCHIVE_FATAL);
	}
	isoent->children_sorted = children;

	ARCHIVE_RB_TREE_FOREACH(rn, &(idr->rbtree)) {
		struct idrent *idrent = (struct idrent *)rn;
		*children ++ = idrent->isoent;
	}
	return (ARCHIVE_OK);
}