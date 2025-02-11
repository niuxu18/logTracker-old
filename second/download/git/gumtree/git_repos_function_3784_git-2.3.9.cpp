xdchange_t *xdl_get_hunk(xdchange_t **xscr, xdemitconf_t const *xecfg)
{
	xdchange_t *xch, *xchp, *lxch;
	long max_common = 2 * xecfg->ctxlen + xecfg->interhunkctxlen;
	long max_ignorable = xecfg->ctxlen;
	unsigned long ignored = 0; /* number of ignored blank lines */

	/* remove ignorable changes that are too far before other changes */
	for (xchp = *xscr; xchp && xchp->ignore; xchp = xchp->next) {
		xch = xchp->next;

		if (xch == NULL ||
		    xch->i1 - (xchp->i1 + xchp->chg1) >= max_ignorable)
			*xscr = xch;
	}

	if (*xscr == NULL)
		return NULL;

	lxch = *xscr;

	for (xchp = *xscr, xch = xchp->next; xch; xchp = xch, xch = xch->next) {
		long distance = xch->i1 - (xchp->i1 + xchp->chg1);
		if (distance > max_common)
			break;

		if (distance < max_ignorable && (!xch->ignore || lxch == xchp)) {
			lxch = xch;
			ignored = 0;
		} else if (distance < max_ignorable && xch->ignore) {
			ignored += xch->chg2;
		} else if (lxch != xchp &&
			   xch->i1 + ignored - (lxch->i1 + lxch->chg1) > max_common) {
			break;
		} else if (!xch->ignore) {
			lxch = xch;
			ignored = 0;
		} else {
			ignored += xch->chg2;
		}
	}

	return lxch;
}