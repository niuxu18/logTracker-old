	*status_p = -1;
	error(_("corrupt binary patch at line %d: %.*s"),
	      linenr-1, llen-1, buffer);
	return NULL;
}

/*
 * Returns:
 *   -1 in case of error,
 *   the length of the parsed binary patch otherwise
 */
static int parse_binary(char *buffer, unsigned long size, struct patch *patch)
{
	/*
	 * We have read "GIT binary patch\n"; what follows is a line
	 * that says the patch method (currently, either "literal" or
	 * "delta") and the length of data before deflating; a
