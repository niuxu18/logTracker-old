static int parse_chunk(struct apply_state *state, char *buffer, unsigned long size, struct patch *patch)
{
	int hdrsize, patchsize;
	int offset = find_header(state, buffer, size, &hdrsize, patch);

	if (offset < 0)
		return offset;

	prefix_patch(state, patch);

	if (!use_patch(state, patch))
		patch->ws_rule = 0;
	else
		patch->ws_rule = whitespace_rule(patch->new_name
						 ? patch->new_name
						 : patch->old_name);

	patchsize = parse_single_patch(state,
				       buffer + offset + hdrsize,
				       size - offset - hdrsize,
				       patch);

	if (patchsize < 0)
		return -128;

	if (!patchsize) {
		static const char git_binary[] = "GIT binary patch\n";
		int hd = hdrsize + offset;
		unsigned long llen = linelen(buffer + hd, size - hd);

		if (llen == sizeof(git_binary) - 1 &&
		    !memcmp(git_binary, buffer + hd, llen)) {
			int used;
			state->linenr++;
			used = parse_binary(state, buffer + hd + llen,
					    size - hd - llen, patch);
			if (used < 0)
				return -1;
			if (used)
				patchsize = used + llen;
			else
				patchsize = 0;
		}
		else if (!memcmp(" differ\n", buffer + hd + llen - 8, 8)) {
			static const char *binhdr[] = {
				"Binary files ",
				"Files ",
				NULL,
			};
			int i;
			for (i = 0; binhdr[i]; i++) {
				int len = strlen(binhdr[i]);
				if (len < size - hd &&
				    !memcmp(binhdr[i], buffer + hd, len)) {
					state->linenr++;
					patch->is_binary = 1;
					patchsize = llen;
					break;
				}
			}
		}

		/* Empty patch cannot be applied if it is a text patch
		 * without metadata change.  A binary patch appears
		 * empty to us here.
		 */
		if ((state->apply || state->check) &&
		    (!patch->is_binary && !metadata_changes(patch))) {
			error(_("patch with only garbage at line %d"), state->linenr);
			return -128;
		}
	}

	return offset + hdrsize + patchsize;
}