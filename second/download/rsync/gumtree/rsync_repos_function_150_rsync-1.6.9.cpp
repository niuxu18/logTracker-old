local int deflate_slow(s, flush)
    deflate_state *s;
    int flush;
{
    IPos hash_head = NIL;    /* head of hash chain */
    int bflush;              /* set if current block must be flushed */

    /* Process the input block. */
    for (;;) {
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead < MIN_LOOKAHEAD && flush == Z_NO_FLUSH) return 1;

            if (s->lookahead == 0) break; /* flush the current block */
        }

        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }

	if (flush == Z_INSERT_ONLY) {
            s->strstart++;
            s->lookahead--;
	    continue;
	}

        /* Find the longest match, discarding those <= prev_length.
         */
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH-1;

        if (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            if (s->strategy != Z_HUFFMAN_ONLY) {
                s->match_length = longest_match (s, hash_head);
            }
            /* longest_match() sets match_start */
            if (s->match_length > s->lookahead) s->match_length = s->lookahead;

            if (s->match_length <= 5 && (s->strategy == Z_FILTERED ||
                 (s->match_length == MIN_MATCH &&
                  s->strstart - s->match_start > TOO_FAR))) {

                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                s->match_length = MIN_MATCH-1;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            uInt max_insert = s->strstart + s->lookahead - MIN_MATCH;
            /* Do not insert strings in hash table beyond this. */

            check_match(s, s->strstart-1, s->prev_match, s->prev_length);

            bflush = ct_tally(s, s->strstart -1 - s->prev_match,
                              s->prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            s->lookahead -= s->prev_length-1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH-1;
            s->strstart++;

            if (bflush) FLUSH_BLOCK(s, Z_NO_FLUSH);

        } else if (s->match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c", s->window[s->strstart-1]));
            if (ct_tally (s, 0, s->window[s->strstart-1])) {
                FLUSH_BLOCK_ONLY(s, Z_NO_FLUSH);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return 1;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    if (flush == Z_INSERT_ONLY) {
	s->block_start = s->strstart;
	return 1;
    }
    Assert (flush != Z_NO_FLUSH, "no flush?");
    if (s->match_available) {
        Tracevv((stderr,"%c", s->window[s->strstart-1]));
        ct_tally (s, 0, s->window[s->strstart-1]);
        s->match_available = 0;
    }
    FLUSH_BLOCK(s, flush);
    return 0;
}