fputs(
"              server.  Quote commands are sent BEFORE the transfer takes place\n"
"              (just after the initial PWD command in an FTP  transfer,  to  be\n"
"              exact). To make commands take place after a successful transfer,\n"
"              prefix them with a dash '-'.  To make  commands  be  sent  after\n"
"              libcurl  has  changed  the  working  directory,  just before the\n"
"              transfer command(s), prefix the command with a '+' (this is only\n"
, stdout);