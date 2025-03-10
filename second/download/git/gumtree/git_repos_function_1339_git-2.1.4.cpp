static void add_mapping(struct string_list *map,
			char *new_name, char *new_email,
			char *old_name, char *old_email)
{
	struct mailmap_entry *me;
	int index;

	if (old_email == NULL) {
		old_email = new_email;
		new_email = NULL;
	}

	if ((index = string_list_find_insert_index(map, old_email, 1)) < 0) {
		/* mailmap entry exists, invert index value */
		index = -1 - index;
		me = (struct mailmap_entry *)map->items[index].util;
	} else {
		/* create mailmap entry */
		struct string_list_item *item;

		item = string_list_insert_at_index(map, index, old_email);
		me = xcalloc(1, sizeof(struct mailmap_entry));
		me->namemap.strdup_strings = 1;
		me->namemap.cmp = namemap_cmp;
		item->util = me;
	}

	if (old_name == NULL) {
		debug_mm("mailmap: adding (simple) entry for %s at index %d\n",
			 old_email, index);
		/* Replace current name and new email for simple entry */
		if (new_name) {
			free(me->name);
			me->name = xstrdup(new_name);
		}
		if (new_email) {
			free(me->email);
			me->email = xstrdup(new_email);
		}
	} else {
		struct mailmap_info *mi = xcalloc(1, sizeof(struct mailmap_info));
		debug_mm("mailmap: adding (complex) entry for %s at index %d\n",
			 old_email, index);
		if (new_name)
			mi->name = xstrdup(new_name);
		if (new_email)
			mi->email = xstrdup(new_email);
		string_list_insert(&me->namemap, old_name)->util = mi;
	}

	debug_mm("mailmap:  '%s' <%s> -> '%s' <%s>\n",
		 debug_str(old_name), old_email,
		 debug_str(new_name), debug_str(new_email));
}