	if (!p) out_of_memory("add_exclude_line");
	for (tok=strtok(p," "); tok; tok=strtok(NULL," "))
		add_exclude(tok, 0);
	free(p);
}


static char *cvs_ignore_list[] = {
  "RCS","SCCS","CVS","CVS.adm","RCSLOG","cvslog.*",
  "tags","TAGS",".make.state",".nse_depinfo",
  "*~", "#*", ".#*", ",*", "*.old", "*.bak", "*.BAK", "*.orig",
  "*.rej", ".del-*", "*.a", "*.o", "*.obj", "*.so", "*.Z", "*.elc", "*.ln",
