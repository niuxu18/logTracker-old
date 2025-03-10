void *hashtable_find(struct hashtable *tbl, int64 key, int allocate_if_missing)
{
	int key64 = tbl->key64;
	struct ht_int32_node *node;
	uint32 ndx;

	if (key64 ? key == 0 : (int32)key == 0) {
		rprintf(FERROR, "Internal hashtable error: illegal key supplied!\n");
		exit_cleanup(RERR_MESSAGEIO);
	}

	if (allocate_if_missing && tbl->entries > HASH_LOAD_LIMIT(tbl->size)) {
		void *old_nodes = tbl->nodes;
		int size = tbl->size * 2;
		int i;

		if (!(tbl->nodes = new_array0(char, size * tbl->node_size)))
			out_of_memory("hashtable_node");
		tbl->size = size;
		tbl->entries = 0;

		if (DEBUG_GTE(HASH, 1)) {
			rprintf(FINFO, "[%s] growing hashtable %lx (size: %d, keys: %d-bit)\n",
				who_am_i(), (long)tbl, size, key64 ? 64 : 32);
		}

		for (i = size / 2; i-- > 0; ) {
			struct ht_int32_node *move_node = HT_NODE(tbl, old_nodes, i);
			int64 move_key = HT_KEY(move_node, key64);
			if (move_key == 0)
				continue;
			node = hashtable_find(tbl, move_key, 1);
			node->data = move_node->data;
		}

		free(old_nodes);
	}

	if (!key64) {
		/* Based on Jenkins One-at-a-time hash. */
		uchar buf[4], *keyp = buf;
		int i;

		SIVALu(buf, 0, key);
		for (ndx = 0, i = 0; i < 4; i++) {
			ndx += keyp[i];
			ndx += (ndx << 10);
			ndx ^= (ndx >> 6);
		}
		ndx += (ndx << 3);
		ndx ^= (ndx >> 11);
		ndx += (ndx << 15);
	} else {
		/* Based on Jenkins hashword() from lookup3.c. */
		uint32 a, b, c;

		/* Set up the internal state */
		a = b = c = 0xdeadbeef + (8 << 2);

#define rot(x,k) (((x)<<(k)) ^ ((x)>>(32-(k))))
#if SIZEOF_INT64 >= 8
		b += (uint32)(key >> 32);
#endif
		a += (uint32)key;
		c ^= b; c -= rot(b, 14);
		a ^= c; a -= rot(c, 11);
		b ^= a; b -= rot(a, 25);
		c ^= b; c -= rot(b, 16);
		a ^= c; a -= rot(c, 4);
		b ^= a; b -= rot(a, 14);
		c ^= b; c -= rot(b, 24);
#undef rot
		ndx = c;
	}

	/* If it already exists, return the node.  If we're not
	 * allocating, return NULL if the key is not found. */
	while (1) {
		int64 nkey;

		ndx &= tbl->size - 1;
		node = HT_NODE(tbl, tbl->nodes, ndx);
		nkey = HT_KEY(node, key64);

		if (nkey == key)
			return node;
		if (nkey == 0) {
			if (!allocate_if_missing)
				return NULL;
			break;
		}
		ndx++;
	}

	/* Take over this empty spot and then return the node. */
	if (key64)
		((struct ht_int64_node*)node)->key = key;
	else
		node->key = (int32)key;
	tbl->entries++;
	return node;
}