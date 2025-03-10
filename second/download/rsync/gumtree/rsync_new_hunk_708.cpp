
	stats.flist_size = stats.total_read - start_read;
	stats.num_files = flist->count;

	return flist;

oom:
	out_of_memory("recv_file_list");
	return NULL;		/* not reached */
}


int file_compare(struct file_struct **file1, struct file_struct **file2)
{
	struct file_struct *f1 = *file1;
	struct file_struct *f2 = *file2;

	if (!f1->basename && !f2->basename)
		return 0;
	if (!f1->basename)
		return -1;
	if (!f2->basename)
		return 1;
	if (f1->dirname == f2->dirname)
		return u_strcmp(f1->basename, f2->basename);
	return f_name_cmp(f1, f2);
}


int flist_find(struct file_list *flist, struct file_struct *f)
{
	int low = 0, high = flist->count - 1;
