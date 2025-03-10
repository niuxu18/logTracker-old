		break;
	default:
		die(_("Ambiguous object name: '%s'."), start_name);
		break;
	}

	if ((commit = lookup_commit_reference(sha1)) == NULL)
		die(_("Not a valid branch point: '%s'."), start_name);
	hashcpy(sha1, commit->object.oid.hash);

	if (reflog)
		log_all_ref_updates = LOG_REFS_NORMAL;

	if (!dont_change_ref) {
		struct ref_transaction *transaction;
