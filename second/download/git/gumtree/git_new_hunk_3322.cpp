			error(ERRORMSG(o, e), path.buf);
			strbuf_release(&path);
		}
		string_list_clear(rejects, 0);
	}
	if (something_displayed)
		fprintf(stderr, _("Aborting\n"));
}

/*
 * Unlink the last component and schedule the leading directories for
 * removal, such that empty directories get removed.
 */
