static ucs_char_t next_hfs_char(const char **in)
{
	while (1) {
		ucs_char_t out = pick_one_utf8_char(in, NULL);
		/*
		 * check for malformed utf8. Technically this
		 * gets converted to a percent-sequence, but
		 * returning 0 is good enough for is_hfs_dotgit
		 * to realize it cannot be .git
		 */
		if (!*in)
			return 0;

		/* these code points are ignored completely */
		switch (out) {
		case 0x200c: /* ZERO WIDTH NON-JOINER */
		case 0x200d: /* ZERO WIDTH JOINER */
		case 0x200e: /* LEFT-TO-RIGHT MARK */
		case 0x200f: /* RIGHT-TO-LEFT MARK */
		case 0x202a: /* LEFT-TO-RIGHT EMBEDDING */
		case 0x202b: /* RIGHT-TO-LEFT EMBEDDING */
		case 0x202c: /* POP DIRECTIONAL FORMATTING */
		case 0x202d: /* LEFT-TO-RIGHT OVERRIDE */
		case 0x202e: /* RIGHT-TO-LEFT OVERRIDE */
		case 0x206a: /* INHIBIT SYMMETRIC SWAPPING */
		case 0x206b: /* ACTIVATE SYMMETRIC SWAPPING */
		case 0x206c: /* INHIBIT ARABIC FORM SHAPING */
		case 0x206d: /* ACTIVATE ARABIC FORM SHAPING */
		case 0x206e: /* NATIONAL DIGIT SHAPES */
		case 0x206f: /* NOMINAL DIGIT SHAPES */
		case 0xfeff: /* ZERO WIDTH NO-BREAK SPACE */
			continue;
		}

		/*
		 * there's a great deal of other case-folding that occurs,
		 * but this is enough to catch anything that will convert
		 * to ".git"
		 */
		return tolower(out);
	}
}