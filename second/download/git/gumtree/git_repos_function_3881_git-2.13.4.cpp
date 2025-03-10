static int init_patch_id_entry(struct patch_id *patch,
			       struct commit *commit,
			       struct patch_ids *ids)
{
	unsigned char header_only_patch_id[GIT_MAX_RAWSZ];

	patch->commit = commit;
	if (commit_patch_id(commit, &ids->diffopts, header_only_patch_id, 1))
		return -1;

	hashmap_entry_init(patch, sha1hash(header_only_patch_id));
	return 0;
}