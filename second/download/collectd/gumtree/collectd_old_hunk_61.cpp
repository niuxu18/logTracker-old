		if (strcasecmp (fields[0], "getval") == 0)
		{
			us_handle_getval (fh, fields, fields_num);
		}
		else if (strcasecmp (fields[0], "putval") == 0)
		{
			us_handle_putval (fh, fields, fields_num);
		}
		else
		{
			fprintf (fh, "-1 Unknown command: %s\n", fields[0]);
			fflush (fh);
		}
