int parse_opt_merge_filter(const struct option *opt, const char *arg, int unset)
{
	struct ref_filter *rf = opt->value;
	unsigned char sha1[20];
	int no_merged = starts_with(opt->long_name, "no");

	if (rf->merge) {
		if (no_merged) {
			return opterror(opt, "is incompatible with --merged", 0);
		} else {
			return opterror(opt, "is incompatible with --no-merged", 0);
		}
	}

	rf->merge = no_merged
		? REF_FILTER_MERGED_OMIT
		: REF_FILTER_MERGED_INCLUDE;

	if (get_sha1(arg, sha1))
		die(_("malformed object name %s"), arg);

	rf->merge_commit = lookup_commit_reference_gently(sha1, 0);
	if (!rf->merge_commit)
		return opterror(opt, "must point to a commit", 0);

	return 0;
}