{
		hash[ 9] ^= le2me_64(block[ 9]);
		hash[10] ^= le2me_64(block[10]);
		hash[11] ^= le2me_64(block[11]);
		hash[12] ^= le2me_64(block[12]);
		/* if not sha3-384 */
		if (block_size > 104) {
			hash[13] ^= le2me_64(block[13]);
			hash[14] ^= le2me_64(block[14]);
			hash[15] ^= le2me_64(block[15]);
			hash[16] ^= le2me_64(block[16]);
			/* if not sha3-256 */
			if (block_size > 136) {
				hash[17] ^= le2me_64(block[17]);
#ifdef FULL_SHA3_FAMILY_SUPPORT
				/* if not sha3-224 */
				if (block_size > 144) {
					hash[18] ^= le2me_64(block[18]);
					hash[19] ^= le2me_64(block[19]);
					hash[20] ^= le2me_64(block[20]);
					hash[21] ^= le2me_64(block[21]);
					hash[22] ^= le2me_64(block[22]);
					hash[23] ^= le2me_64(block[23]);
					hash[24] ^= le2me_64(block[24]);
				}
#endif
			}
		}
	}