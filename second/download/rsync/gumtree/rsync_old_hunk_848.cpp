			gidset[ngroups++] = mygid;
		if (verbose > 3) {
			int pos;
			char *gidbuf = new_array(char, ngroups*21+32);
			if (!gidbuf)
				out_of_memory("is_in_group");
			sprintf(gidbuf, "process has %d gid%s: ",
			    ngroups, ngroups == 1? "" : "s");
			pos = strlen(gidbuf);
			for (n = 0; n < ngroups; n++) {
				sprintf(gidbuf+pos, " %d", (int)gidset[n]);
				pos += strlen(gidbuf+pos);
			}
			rprintf(FINFO, "%s\n", gidbuf);
			free(gidbuf);
		}
	}

