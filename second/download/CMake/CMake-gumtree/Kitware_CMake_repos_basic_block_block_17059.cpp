{
	char buff[64];
	const char *p;

	/* Handle the null filename case. */
	if (src == NULL || *src == '\0') {
		strcpy(dest, "PaxHeader/blank");
		return (dest);
	}

	/* Prune final '/' and other unwanted final elements. */
	p = src + strlen(src);
	for (;;) {
		/* Ends in "/", remove the '/' */
		if (p > src && p[-1] == '/') {
			--p;
			continue;
		}
		/* Ends in "/.", remove the '.' */
		if (p > src + 1 && p[-1] == '.'
		    && p[-2] == '/') {
			--p;
			continue;
		}
		break;
	}

	/* Pathological case: After above, there was nothing left.
	 * This includes "/." "/./." "/.//./." etc. */
	if (p == src) {
		strcpy(dest, "/PaxHeader/rootdir");
		return (dest);
	}

	/* Convert unadorned "." into a suitable filename. */
	if (*src == '.' && p == src + 1) {
		strcpy(dest, "PaxHeader/currentdir");
		return (dest);
	}

	/*
	 * TODO: Push this string into the 'pax' structure to avoid
	 * recomputing it every time.  That will also open the door
	 * to having clients override it.
	 */
#if HAVE_GETPID && 0  /* Disable this for now; see above comment. */
	sprintf(buff, "PaxHeader.%d", getpid());
#else
	/* If the platform can't fetch the pid, don't include it. */
	strcpy(buff, "PaxHeader");
#endif
	/* General case: build a ustar-compatible name adding
	 * "/PaxHeader/". */
	build_ustar_entry_name(dest, src, p - src, buff);

	return (dest);
}