int poptParseArgvString(const unsigned char * s, int * argcPtr, const char *** argvPtr)
{
    const unsigned char * src;
    unsigned char quote = '\0';
    int argvAlloced = POPT_ARGV_ARRAY_GROW_DELTA;
    const char ** argv = malloc(sizeof(*argv) * argvAlloced);
    int argc = 0;
    int buflen = strlen(s) + 1;
    char * buf = memset(alloca(buflen), 0, buflen);
    int rc = POPT_ERROR_MALLOC;

    if (argv == NULL) return rc;
    argv[argc] = buf;

    for (src = s; *src != '\0'; src++) {
	if (quote == *src) {
	    quote = '\0';
	} else if (quote != '\0') {
	    if (*src == '\\') {
		src++;
		if (!*src) {
		    rc = POPT_ERROR_BADQUOTE;
		    goto exit;
		}
		if (*src != quote) *buf++ = '\\';
	    }
	    *buf++ = *src;
	} else if (isspace(*src)) {
	    if (*argv[argc] != '\0') {
		buf++, argc++;
		if (argc == argvAlloced) {
		    argvAlloced += POPT_ARGV_ARRAY_GROW_DELTA;
		    argv = realloc(argv, sizeof(*argv) * argvAlloced);
		    if (argv == NULL) goto exit;
		}
		argv[argc] = buf;
	    }
	} else switch (*src) {
	  case '"':
	  case '\'':
	    quote = *src;
	    /*@switchbreak@*/ break;
	  case '\\':
	    src++;
	    if (!*src) {
		rc = POPT_ERROR_BADQUOTE;
		goto exit;
	    }
	    /*@fallthrough@*/
	  default:
	    *buf++ = *src;
	    /*@switchbreak@*/ break;
	}
    }

    if (strlen(argv[argc])) {
	argc++, buf++;
    }

    rc = poptDupArgv(argc, argv, argcPtr, argvPtr);

exit:
    if (argv) free(argv);
    return rc;
}