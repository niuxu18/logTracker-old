static void parse_options_check(const struct option *opts)
{
	int err = 0;

	for (; opts->type != OPTION_END; opts++) {
		if ((opts->flags & PARSE_OPT_LASTARG_DEFAULT) &&
		    (opts->flags & PARSE_OPT_OPTARG))
			err |= optbug(opts, "uses incompatible flags "
					"LASTARG_DEFAULT and OPTARG");
		if (opts->flags & PARSE_OPT_NODASH &&
		    ((opts->flags & PARSE_OPT_OPTARG) ||
		     !(opts->flags & PARSE_OPT_NOARG) ||
		     !(opts->flags & PARSE_OPT_NONEG) ||
		     opts->long_name))
			err |= optbug(opts, "uses feature "
					"not supported for dashless options");
		switch (opts->type) {
		case OPTION_COUNTUP:
		case OPTION_BIT:
		case OPTION_NEGBIT:
		case OPTION_SET_INT:
		case OPTION_NUMBER:
			if ((opts->flags & PARSE_OPT_OPTARG) ||
			    !(opts->flags & PARSE_OPT_NOARG))
				err |= optbug(opts, "should not accept an argument");
		default:
			; /* ok. (usually accepts an argument) */
		}
		if (opts->argh &&
		    strcspn(opts->argh, " _") != strlen(opts->argh))
			err |= optbug(opts, "multi-word argh should use dash to separate words");
	}
	if (err)
		exit(128);
}