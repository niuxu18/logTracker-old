	}
	strbuf_release(&buf);
}

int git_default_advice_config(const char *var, const char *value)
{
	const char *k = skip_prefix(var, "advice.");
	int i;

	for (i = 0; i < ARRAY_SIZE(advice_config); i++) {
		if (strcmp(k, advice_config[i].name))
			continue;
		*advice_config[i].preference = git_config_bool(var, value);
		return 0;
	}

	return 0;
}

int error_resolve_conflict(const char *me)
{
	error("'%s' is not possible because you have unmerged files.", me);
	if (advice_resolve_conflict)
		/*
		 * Message used both when 'git commit' fails and when
		 * other commands doing a merge do.
		 */
		advise(_("Fix them up in the work tree,\n"
			 "and then use 'git add/rm <file>' as\n"
			 "appropriate to mark resolution and make a commit,\n"
			 "or use 'git commit -a'."));
	return -1;
}

void NORETURN die_resolve_conflict(const char *me)
{
	error_resolve_conflict(me);
