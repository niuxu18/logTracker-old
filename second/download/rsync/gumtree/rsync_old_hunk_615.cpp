		}

		if (strcmp(line,"@RSYNCD: OK") == 0) break;

		if (strcmp(line,"@RSYNCD: EXIT") == 0) exit(0);

		rprintf(FINFO,"%s\n", line);
	}
	kludge_around_eof = False;

	for (i=0;i<sargc;i++) {
		io_printf(fd,"%s\n", sargs[i]);
	}
