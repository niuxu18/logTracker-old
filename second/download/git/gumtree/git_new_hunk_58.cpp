			item->util = PATH_TO_BE_DELETED;
		}
		patch = patch->next;
	}
}

static int checkout_target(struct index_state *istate,
			   struct cache_entry *ce, struct stat *st)
{
	struct checkout costate;

	memset(&costate, 0, sizeof(costate));
	costate.base_dir = "";
	costate.refresh_cache = 1;
	costate.istate = istate;
	if (checkout_entry(ce, &costate, NULL) || lstat(ce->name, st))
		return error(_("cannot checkout %s"), ce->name);
	return 0;
}

static struct patch *previous_patch(struct patch *patch, int *gone)
