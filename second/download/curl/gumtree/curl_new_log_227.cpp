puts(
"             -F \"filedescription=Cool text file with cool text inside\" \\\n"
"             http://www.post.com/postit.cgi\n"
"\n"
"  To send two files in one post you can do it in two ways:\n"
"\n"
"  1. Send multiple files in a single \"field\" with a single field name:\n"
" \n"
"        curl -F \"pictures=@dog.gif,cat.gif\" \n"
" \n"
"  2. Send two fields with two field names: \n"
"\n"
"        curl -F \"docpicture=@dog.gif\" -F \"catpicture=@cat.gif\" \n"
"\n"
"REFERRER\n"
"\n"
"  A HTTP request has the option to include information about which address\n"
);