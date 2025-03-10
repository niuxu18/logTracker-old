static void hash_search(int f,struct sum_struct *s,
			struct map_struct *buf, OFF_T len)
{
	OFF_T offset, end, backup;
	int32 k, want_i;
	char sum2[SUM_LENGTH];
	uint32 s1, s2, sum;
	int more;
	schar *map;

	/* want_i is used to encourage adjacent matches, allowing the RLL
	 * coding of the output to work more efficiently. */
	want_i = 0;

	if (verbose > 2) {
		rprintf(FINFO, "hash search b=%ld len=%.0f\n",
			(long)s->blength, (double)len);
	}

	k = (int32)MIN(len, (OFF_T)s->blength);

	map = (schar *)map_ptr(buf, 0, k);

	sum = get_checksum1((char *)map, k);
	s1 = sum & 0xFFFF;
	s2 = sum >> 16;
	if (verbose > 3)
		rprintf(FINFO, "sum=%.8x k=%ld\n", sum, (long)k);

	offset = 0;

	end = len + 1 - s->sums[s->count-1].len;

	if (verbose > 3) {
		rprintf(FINFO, "hash search s->blength=%ld len=%.0f count=%.0f\n",
			(long)s->blength, (double)len, (double)s->count);
	}

	do {
		tag t = gettag2(s1,s2);
		int done_csum2 = 0;
		int32 j = tag_table[t];

		if (verbose > 4)
			rprintf(FINFO,"offset=%.0f sum=%08x\n",(double)offset,sum);

		if (j == NULL_TAG)
			goto null_tag;

		sum = (s1 & 0xffff) | (s2 << 16);
		tag_hits++;
		do {
			int32 l, i = targets[j].i;

			if (sum != s->sums[i].sum1)
				continue;

			/* also make sure the two blocks are the same length */
			l = (int32)MIN((OFF_T)s->blength, len-offset);
			if (l != s->sums[i].len)
				continue;

			/* in-place: ensure chunk's offset is either >= our
			 * offset or that the data didn't move. */
			if (updating_basis_file && s->sums[i].offset < offset
			    && !(s->sums[i].flags & SUMFLG_SAME_OFFSET))
				continue;

			if (verbose > 3)
				rprintf(FINFO,"potential match at %.0f target=%.0f %.0f sum=%08x\n",
					(double)offset,(double)j,(double)i,sum);

			if (!done_csum2) {
				map = (schar *)map_ptr(buf,offset,l);
				get_checksum2((char *)map,l,sum2);
				done_csum2 = 1;
			}

			if (memcmp(sum2,s->sums[i].sum2,s->s2length) != 0) {
				false_alarms++;
				continue;
			}

			/* When updating in-place, the best possible match is
			 * one with an identical offset, so we prefer that over
			 * the following want_i optimization. */
			if (updating_basis_file) {
				do {
					int32 i2 = targets[j].i;
					if (s->sums[i2].offset != offset)
						continue;
					if (i2 != i) {
						if (sum != s->sums[i2].sum1)
							break;
						if (memcmp(sum2, s->sums[i2].sum2,
							   s->s2length) != 0)
							break;
						i = i2;
					}
					/* This chunk was at the same offset on
					 * both the sender and the receiver. */
					s->sums[i].flags |= SUMFLG_SAME_OFFSET;
					goto set_want_i;
				} while (++j < s->count && targets[j].t == t);
			}

			/* we've found a match, but now check to see
			 * if want_i can hint at a better match. */
			if (i != want_i && want_i < s->count
			    && (!updating_basis_file || s->sums[want_i].offset >= offset
			     || s->sums[want_i].flags & SUMFLG_SAME_OFFSET)
			    && sum == s->sums[want_i].sum1
			    && memcmp(sum2, s->sums[want_i].sum2, s->s2length) == 0) {
				/* we've found an adjacent match - the RLL coder
				 * will be happy */
				i = want_i;
			}
		    set_want_i:
			want_i = i + 1;

			matched(f,s,buf,offset,i);
			offset += s->sums[i].len - 1;
			k = (int32)MIN((OFF_T)s->blength, len-offset);
			map = (schar *)map_ptr(buf, offset, k);
			sum = get_checksum1((char *)map, k);
			s1 = sum & 0xFFFF;
			s2 = sum >> 16;
			matches++;
			break;
		} while (++j < s->count && targets[j].t == t);

	null_tag:
		backup = offset - last_match;
		/* We sometimes read 1 byte prior to last_match... */
		if (backup < 0)
			backup = 0;

		/* Trim off the first byte from the checksum */
		more = offset + k < len;
		map = (schar *)map_ptr(buf, offset - backup, k + more + backup)
		    + backup;
		s1 -= map[0] + CHAR_OFFSET;
		s2 -= k * (map[0]+CHAR_OFFSET);

		/* Add on the next byte (if there is one) to the checksum */
		if (more) {
			s1 += map[k] + CHAR_OFFSET;
			s2 += s1;
		} else
			--k;

		/* By matching early we avoid re-reading the
		   data 3 times in the case where a token
		   match comes a long way after last
		   match. The 3 reads are caused by the
		   running match, the checksum update and the
		   literal send. */
		if (backup >= s->blength+CHUNK_SIZE && end-offset > CHUNK_SIZE)
			matched(f, s, buf, offset - s->blength, -2);
	} while (++offset < end);

	matched(f, s, buf, len, -1);
	map_ptr(buf, len-1, 1);
}