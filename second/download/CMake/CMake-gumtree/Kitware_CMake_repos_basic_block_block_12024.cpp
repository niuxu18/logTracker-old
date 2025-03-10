{
		case ST_RD_BLOCK:
			/*
			 * Read a block number indicates how many blocks
			 * we will handle. The block is composed of a
			 * literal and a match, sometimes a literal only
			 * in particular, there are no reference data at
			 * the beginning of the decompression.
			 */
			if (!lzh_br_read_ahead_0(strm, br, 16)) {
				if (!last)
					/* We need following data. */
					return (ARCHIVE_OK);
				if (lzh_br_has(br, 8)) {
					/*
					 * It seems there are extra bits.
					 *  1. Compressed data is broken.
					 *  2. `last' flag does not properly
					 *     set.
					 */
					goto failed;
				}
				if (ds->w_pos > 0) {
					lzh_emit_window(strm, ds->w_pos);
					ds->w_pos = 0;
					return (ARCHIVE_OK);
				}
				/* End of compressed data; we have completely
				 * handled all compressed data. */
				return (ARCHIVE_EOF);
			}
			ds->blocks_avail = lzh_br_bits(br, 16);
			if (ds->blocks_avail == 0)
				goto failed;
			lzh_br_consume(br, 16);
			/*
			 * Read a literal table compressed in huffman
			 * coding.
			 */
			ds->pt.len_size = ds->literal_pt_len_size;
			ds->pt.len_bits = ds->literal_pt_len_bits;
			ds->reading_position = 0;
			/* FALL THROUGH */
		case ST_RD_PT_1:
			/* Note: ST_RD_PT_1, ST_RD_PT_2 and ST_RD_PT_4 are
			 * used in reading both a literal table and a
			 * position table. */
			if (!lzh_br_read_ahead(strm, br, ds->pt.len_bits)) {
				if (last)
					goto failed;/* Truncated data. */
				ds->state = ST_RD_PT_1;
				return (ARCHIVE_OK);
			}
			ds->pt.len_avail = lzh_br_bits(br, ds->pt.len_bits);
			lzh_br_consume(br, ds->pt.len_bits);
			/* FALL THROUGH */
		case ST_RD_PT_2:
			if (ds->pt.len_avail == 0) {
				/* There is no bitlen. */
				if (!lzh_br_read_ahead(strm, br,
				    ds->pt.len_bits)) {
					if (last)
						goto failed;/* Truncated data.*/
					ds->state = ST_RD_PT_2;
					return (ARCHIVE_OK);
				}
				if (!lzh_make_fake_table(&(ds->pt),
				    lzh_br_bits(br, ds->pt.len_bits)))
					goto failed;/* Invalid data. */
				lzh_br_consume(br, ds->pt.len_bits);
				if (ds->reading_position)
					ds->state = ST_GET_LITERAL;
				else
					ds->state = ST_RD_LITERAL_1;
				break;
			} else if (ds->pt.len_avail > ds->pt.len_size)
				goto failed;/* Invalid data. */
			ds->loop = 0;
			memset(ds->pt.freq, 0, sizeof(ds->pt.freq));
			if (ds->pt.len_avail < 3 ||
			    ds->pt.len_size == ds->pos_pt_len_size) {
				ds->state = ST_RD_PT_4;
				break;
			}
			/* FALL THROUGH */
		case ST_RD_PT_3:
			ds->loop = lzh_read_pt_bitlen(strm, ds->loop, 3);
			if (ds->loop < 3) {
				if (ds->loop < 0 || last)
					goto failed;/* Invalid data. */
				/* Not completed, get following data. */
				ds->state = ST_RD_PT_3;
				return (ARCHIVE_OK);
			}
			/* There are some null in bitlen of the literal. */
			if (!lzh_br_read_ahead(strm, br, 2)) {
				if (last)
					goto failed;/* Truncated data. */
				ds->state = ST_RD_PT_3;
				return (ARCHIVE_OK);
			}
			c = lzh_br_bits(br, 2);
			lzh_br_consume(br, 2);
			if (c > ds->pt.len_avail - 3)
				goto failed;/* Invalid data. */
			for (i = 3; c-- > 0 ;)
				ds->pt.bitlen[i++] = 0;
			ds->loop = i;
			/* FALL THROUGH */
		case ST_RD_PT_4:
			ds->loop = lzh_read_pt_bitlen(strm, ds->loop,
			    ds->pt.len_avail);
			if (ds->loop < ds->pt.len_avail) {
				if (ds->loop < 0 || last)
					goto failed;/* Invalid data. */
				/* Not completed, get following data. */
				ds->state = ST_RD_PT_4;
				return (ARCHIVE_OK);
			}
			if (!lzh_make_huffman_table(&(ds->pt)))
				goto failed;/* Invalid data */
			if (ds->reading_position) {
				ds->state = ST_GET_LITERAL;
				break;
			}
			/* FALL THROUGH */
		case ST_RD_LITERAL_1:
			if (!lzh_br_read_ahead(strm, br, ds->lt.len_bits)) {
				if (last)
					goto failed;/* Truncated data. */
				ds->state = ST_RD_LITERAL_1;
				return (ARCHIVE_OK);
			}
			ds->lt.len_avail = lzh_br_bits(br, ds->lt.len_bits);
			lzh_br_consume(br, ds->lt.len_bits);
			/* FALL THROUGH */
		case ST_RD_LITERAL_2:
			if (ds->lt.len_avail == 0) {
				/* There is no bitlen. */
				if (!lzh_br_read_ahead(strm, br,
				    ds->lt.len_bits)) {
					if (last)
						goto failed;/* Truncated data.*/
					ds->state = ST_RD_LITERAL_2;
					return (ARCHIVE_OK);
				}
				if (!lzh_make_fake_table(&(ds->lt),
				    lzh_br_bits(br, ds->lt.len_bits)))
					goto failed;/* Invalid data */
				lzh_br_consume(br, ds->lt.len_bits);
				ds->state = ST_RD_POS_DATA_1;
				break;
			} else if (ds->lt.len_avail > ds->lt.len_size)
				goto failed;/* Invalid data */
			ds->loop = 0;
			memset(ds->lt.freq, 0, sizeof(ds->lt.freq));
			/* FALL THROUGH */
		case ST_RD_LITERAL_3:
			i = ds->loop;
			while (i < ds->lt.len_avail) {
				if (!lzh_br_read_ahead(strm, br,
				    ds->pt.max_bits)) {
					if (last)
						goto failed;/* Truncated data.*/
					ds->loop = i;
					ds->state = ST_RD_LITERAL_3;
					return (ARCHIVE_OK);
				}
				rbits = lzh_br_bits(br, ds->pt.max_bits);
				c = lzh_decode_huffman(&(ds->pt), rbits);
				if (c > 2) {
					/* Note: 'c' will never be more than
					 * eighteen since it's limited by
					 * PT_BITLEN_SIZE, which is being set
					 * to ds->pt.len_size through
					 * ds->literal_pt_len_size. */
					lzh_br_consume(br, ds->pt.bitlen[c]);
					c -= 2;
					ds->lt.freq[c]++;
					ds->lt.bitlen[i++] = c;
				} else if (c == 0) {
					lzh_br_consume(br, ds->pt.bitlen[c]);
					ds->lt.bitlen[i++] = 0;
				} else {
					/* c == 1 or c == 2 */
					int n = (c == 1)?4:9;
					if (!lzh_br_read_ahead(strm, br,
					     ds->pt.bitlen[c] + n)) {
						if (last) /* Truncated data. */
							goto failed;
						ds->loop = i;
						ds->state = ST_RD_LITERAL_3;
						return (ARCHIVE_OK);
					}
					lzh_br_consume(br, ds->pt.bitlen[c]);
					c = lzh_br_bits(br, n);
					lzh_br_consume(br, n);
					c += (n == 4)?3:20;
					if (i + c > ds->lt.len_avail)
						goto failed;/* Invalid data */
					memset(&(ds->lt.bitlen[i]), 0, c);
					i += c;
				}
			}
			if (i > ds->lt.len_avail ||
			    !lzh_make_huffman_table(&(ds->lt)))
				goto failed;/* Invalid data */
			/* FALL THROUGH */
		case ST_RD_POS_DATA_1:
			/*
			 * Read a position table compressed in huffman
			 * coding.
			 */
			ds->pt.len_size = ds->pos_pt_len_size;
			ds->pt.len_bits = ds->pos_pt_len_bits;
			ds->reading_position = 1;
			ds->state = ST_RD_PT_1;
			break;
		case ST_GET_LITERAL:
			return (100);
		}