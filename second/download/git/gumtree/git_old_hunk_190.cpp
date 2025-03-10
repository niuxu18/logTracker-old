	N_("'git help -a' and 'git help -g' lists available subcommands and some\n"
	   "concept guides. See 'git help <command>' or 'git help <concept>'\n"
	   "to read about a specific subcommand or concept.");

static struct startup_info git_startup_info;
static int use_pager = -1;

static void commit_pager_choice(void) {
	switch (use_pager) {
	case 0:
		setenv("GIT_PAGER", "cat", 1);
		break;
