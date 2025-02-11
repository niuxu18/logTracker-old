(text != NULL && *text != L'\0') {
		/*
		 * Parse the fields out of the next entry,
		 * advance 'text' to start of next entry.
		 */
		fields = 0;
		do {
			const wchar_t *start, *end;
			next_field_w(&text, &start, &end, &sep);
			if (fields < numfields) {
				field[fields].start = start;
				field[fields].end = end;
			}
			++fields;
		} while (sep == L':');

		/* Set remaining fields to blank. */
		for (n = fields; n < numfields; ++n)
			field[n].start = field[n].end = NULL;

		if (field[0].start != NULL && *(field[0].start) == L'#') {
			/* Comment, skip entry */
			continue;
		}

		n = 0;
		sol = 0;
		id = -1;
		permset = 0;
		name.start = name.end = NULL;

		if (want_type != ARCHIVE_ENTRY_ACL_TYPE_NFS4) {
			/* POSIX.1e ACLs */
			/*
			 * Default keyword "default:user::rwx"
			 * if found, we have one more field
			 *
			 * We also support old Solaris extension:
			 * "defaultuser::rwx" is the default ACL corresponding
			 * to "user::rwx", etc. valid only for first field
			 */
			s = field[0].start;
			len = field[0].end - field[0].start;
			if (*s == L'd' && (len == 1 || (len >= 7
			    && wmemcmp((s + 1), L"efault", 6) == 0))) {
				type = ARCHIVE_ENTRY_ACL_TYPE_DEFAULT;
				if (len > 7)
					field[0].start += 7;
				else
					n = 1;
			} else
				type = want_type;

			/* Check for a numeric ID in field n+1 or n+3. */
			isint_w(field[n + 1].start, field[n + 1].end, &id);
			/* Field n+3 is optional. */
			if (id == -1 && fields > n+3)
				isint_w(field[n + 3].start, field[n + 3].end,
				    &id);

			tag = 0;
			s = field[n].start;
			st = field[n].start + 1;
			len = field[n].end - field[n].start;

			switch (*s) {
			case L'u':
				if (len == 1 || (len == 4
				    && wmemcmp(st, L"ser", 3) == 0))
					tag = ARCHIVE_ENTRY_ACL_USER_OBJ;
				break;
			case L'g':
				if (len == 1 || (len == 5
				    && wmemcmp(st, L"roup", 4) == 0))
					tag = ARCHIVE_ENTRY_ACL_GROUP_OBJ;
				break;
			case L'o':
				if (len == 1 || (len == 5
				    && wmemcmp(st, L"ther", 4) == 0))
					tag = ARCHIVE_ENTRY_ACL_OTHER;
				break;
			case L'm':
				if (len == 1 || (len == 4
				    && wmemcmp(st, L"ask", 3) == 0))
					tag = ARCHIVE_ENTRY_ACL_MASK;
				break;
			default:
					break;
			}

			switch (tag) {
			case ARCHIVE_ENTRY_ACL_OTHER:
			case ARCHIVE_ENTRY_ACL_MASK:
				if (fields == (n + 2)
				    && field[n + 1].start < field[n + 1].end
				    && ismode_w(field[n + 1].start,
				    field[n + 1].end, &permset)) {
					/* This is Solaris-style "other:rwx" */
					sol = 1;
				} else if (fields == (n + 3) &&
				    field[n + 1].start < field[n + 1].end) {
					/* Invalid mask or other field */
					ret = ARCHIVE_WARN;
					continue;
				}
				break;
			case ARCHIVE_ENTRY_ACL_USER_OBJ:
			case ARCHIVE_ENTRY_ACL_GROUP_OBJ:
				if (id != -1 ||
				    field[n + 1].start < field[n + 1].end) {
					name = field[n + 1];
					if (tag == ARCHIVE_ENTRY_ACL_USER_OBJ)
						tag = ARCHIVE_ENTRY_ACL_USER;
					else
						tag = ARCHIVE_ENTRY_ACL_GROUP;
				}
				break;
			default:
				/* Invalid tag, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			}

			/*
			 * Without "default:" we expect mode in field 2
			 * Exception: Solaris other and mask fields
			 */
			if (permset == 0 && !ismode_w(field[n + 2 - sol].start,
			    field[n + 2 - sol].end, &permset)) {
				/* Invalid mode, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			}
		} else {
			/* NFS4 ACLs */
			s = field[0].start;
			len = field[0].end - field[0].start;
			tag = 0;

			switch (len) {
			case 4:
				if (wmemcmp(s, L"user", 4) == 0)
					tag = ARCHIVE_ENTRY_ACL_USER;
				break;
			case 5:
				if (wmemcmp(s, L"group", 5) == 0)
					tag = ARCHIVE_ENTRY_ACL_GROUP;
				break;
			case 6:
				if (wmemcmp(s, L"owner@", 6) == 0)
					tag = ARCHIVE_ENTRY_ACL_USER_OBJ;
				else if (wmemcmp(s, L"group@", len) == 0)
					tag = ARCHIVE_ENTRY_ACL_GROUP_OBJ;
				break;
			case 9:
				if (wmemcmp(s, L"everyone@", 9) == 0)
					tag = ARCHIVE_ENTRY_ACL_EVERYONE;
			default:
				break;
			}

			if (tag == 0) {
				/* Invalid tag, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			} else if (tag == ARCHIVE_ENTRY_ACL_USER ||
			    tag == ARCHIVE_ENTRY_ACL_GROUP) {
				n = 1;
				name = field[1];
				isint_w(name.start, name.end, &id);
			} else
				n = 0;

			if (!is_nfs4_perms_w(field[1 + n].start,
			    field[1 + n].end, &permset)) {
				/* Invalid NFSv4 perms, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			}
			if (!is_nfs4_flags_w(field[2 + n].start,
			    field[2 + n].end, &permset)) {
				/* Invalid NFSv4 flags, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			}
			s = field[3 + n].start;
			len = field[3 + n].end - field[3 + n].start;
			type = 0;
			if (len == 4) {
				if (wmemcmp(s, L"deny", 4) == 0)
					type = ARCHIVE_ENTRY_ACL_TYPE_DENY;
			} else if (len == 5) {
				if (wmemcmp(s, L"allow", 5) == 0)
					type = ARCHIVE_ENTRY_ACL_TYPE_ALLOW;
				else if (wmemcmp(s, L"audit", 5) == 0)
					type = ARCHIVE_ENTRY_ACL_TYPE_AUDIT;
				else if (wmemcmp(s, L"alarm", 5) == 0)
					type = ARCHIVE_ENTRY_ACL_TYPE_ALARM;
			}
			if (type == 0) {
				/* Invalid entry type, skip entry */
				ret = ARCHIVE_WARN;
				continue;
			}
			isint_w(field[4 + n].start, field[4 + n].end, &id);
		}

		/* Add entry to the internal list. */
		r = archive_acl_add_entry_w_len(acl, type, permset,
		    tag, id, name.start, name.end - name.start);
		if (r < ARCHIVE_WARN)
			return (r);
		if (r != ARCHIVE_OK)
			ret = ARCHIVE_WARN;
		types |= type;
	}