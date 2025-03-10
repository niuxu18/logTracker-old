static void show_object(struct object *obj, const char *name, void *cb_data)
{
	struct rev_list_info *info = cb_data;
	finish_object(obj, name, cb_data);
	display_progress(progress, ++progress_counter);
	if (info->flags & REV_LIST_QUIET)
		return;
	show_object_with_name(stdout, obj, name);
}