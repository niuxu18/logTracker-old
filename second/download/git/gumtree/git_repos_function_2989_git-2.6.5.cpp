static struct commit_list *skip_away(struct commit_list *list, int count)
{
	struct commit_list *cur, *previous;
	int prn, index, i;

	prn = get_prn(count);
	index = (count * prn / PRN_MODULO) * sqrti(prn) / sqrti(PRN_MODULO);

	cur = list;
	previous = NULL;

	for (i = 0; cur; cur = cur->next, i++) {
		if (i == index) {
			if (hashcmp(cur->item->object.sha1, current_bad_oid->hash))
				return cur;
			if (previous)
				return previous;
			return list;
		}
		previous = cur;
	}

	return list;
}