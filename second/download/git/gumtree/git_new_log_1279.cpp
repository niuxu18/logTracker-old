error(_("Cannot add notes. "
					"Found existing notes for object %s. "
					"Use '-f' to overwrite existing notes"),
					oid_to_hex(&object));