static int xdl_fill_merge_buffer(xdfenv_t *xe1, const char *name1,
				 xdfenv_t *xe2, const char *name2,
				 const char *ancestor_name,
				 int favor,
				 xdmerge_t *m, char *dest, int style,
				 int marker_size)
{
	int size, i;

	for (size = i = 0; m; m = m->next) {
		if (favor && !m->mode)
			m->mode = favor;

		if (m->mode == 0)
			size = fill_conflict_hunk(xe1, name1, xe2, name2,
						  ancestor_name,
						  size, i, style, m, dest,
						  marker_size);
		else if (m->mode & 3) {
			/* Before conflicting part */
			size += xdl_recs_copy(xe1, i, m->i1 - i, 0,
					      dest ? dest + size : NULL);
			/* Postimage from side #1 */
			if (m->mode & 1)
				size += xdl_recs_copy(xe1, m->i1, m->chg1, 1,
						      dest ? dest + size : NULL);
			/* Postimage from side #2 */
			if (m->mode & 2)
				size += xdl_recs_copy(xe2, m->i2, m->chg2, 1,
						      dest ? dest + size : NULL);
		} else
			continue;
		i = m->i1 + m->chg1;
	}
	size += xdl_recs_copy(xe1, i, xe1->xdf2.nrec - i, 0,
			      dest ? dest + size : NULL);
	return size;
}