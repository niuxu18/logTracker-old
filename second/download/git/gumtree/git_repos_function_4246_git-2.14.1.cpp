static void unique_in_pack(struct packed_git *p,
			   struct disambiguate_state *ds)
{
	uint32_t num, last, i, first = 0;
	const struct object_id *current = NULL;

	open_pack_index(p);
	num = p->num_objects;
	last = num;
	while (first < last) {
		uint32_t mid = (first + last) / 2;
		const unsigned char *current;
		int cmp;

		current = nth_packed_object_sha1(p, mid);
		cmp = hashcmp(ds->bin_pfx.hash, current);
		if (!cmp) {
			first = mid;
			break;
		}
		if (cmp > 0) {
			first = mid+1;
			continue;
		}
		last = mid;
	}

	/*
	 * At this point, "first" is the location of the lowest object
	 * with an object name that could match "bin_pfx".  See if we have
	 * 0, 1 or more objects that actually match(es).
	 */
	for (i = first; i < num && !ds->ambiguous; i++) {
		struct object_id oid;
		current = nth_packed_object_oid(&oid, p, i);
		if (!match_sha(ds->len, ds->bin_pfx.hash, current->hash))
			break;
		update_candidates(ds, current);
	}
}