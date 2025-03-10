{
	if (old_stats->crlf && !new_stats->crlf ) {
		/*
		 * CRLFs would not be restored by checkout
		 */
		if (checksafe == SAFE_CRLF_WARN)
			warning(_("CRLF will be replaced by LF in %s.\n"
				  "The file will have its original line"
				  " endings in your working directory."), path);
		else /* i.e. SAFE_CRLF_FAIL */
			die(_("CRLF would be replaced by LF in %s."), path);
	} else if (old_stats->lonelf && !new_stats->lonelf ) {
		/*
		 * CRLFs would be added by checkout
		 */
		if (checksafe == SAFE_CRLF_WARN)
			warning(_("LF will be replaced by CRLF in %s.\n"
				  "The file will have its original line"
				  " endings in your working directory."), path);
		else /* i.e. SAFE_CRLF_FAIL */
			die(_("LF would be replaced by CRLF in %s"), path);
	}
}

static int has_cr_in_index(const char *path)
{
	unsigned long sz;
