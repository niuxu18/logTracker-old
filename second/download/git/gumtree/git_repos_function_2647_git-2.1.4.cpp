static struct commit_list *best_bisection_sorted(struct commit_list *list, int nr)
{
	struct commit_list *p;
	struct commit_dist *array = xcalloc(nr, sizeof(*array));
	int cnt, i;

	for (p = list, cnt = 0; p; p = p->next) {
		int distance;
		unsigned flags = p->item->object.flags;

		if (flags & TREESAME)
			continue;
		distance = weight(p);
		if (nr - distance < distance)
			distance = nr - distance;
		array[cnt].commit = p->item;
		array[cnt].distance = distance;
		cnt++;
	}
	qsort(array, cnt, sizeof(*array), compare_commit_dist);
	for (p = list, i = 0; i < cnt; i++) {
		struct name_decoration *r = xmalloc(sizeof(*r) + 100);
		struct object *obj = &(array[i].commit->object);

		sprintf(r->name, "dist=%d", array[i].distance);
		r->next = add_decoration(&name_decoration, obj, r);
		p->item = array[i].commit;
		p = p->next;
	}
	if (p)
		p->next = NULL;
	free(array);
	return list;
}