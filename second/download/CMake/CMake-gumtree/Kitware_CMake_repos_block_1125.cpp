{
			/* No separator. */
			archive_set_error(&a->archive, ENAMETOOLONG,
			    "Pathname too long");
			ret = ARCHIVE_FAILED;
		}