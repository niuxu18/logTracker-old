fputs(
"   that is unrelated to the login password of the remote system.  If you\n"
"   provide a private key file you must also provide a public key file.\n"
"\n"
" HTTP\n"
"\n"
"   Curl also supports user and password in HTTP URLs, thus you can pick a file\n"
"   like:\n"
"\n"
"        curl http://name:passwd@machine.domain/full/path/to/file\n"
"\n"
"   or specify user and password separately like in\n"
"\n"
"        curl -u name:passwd http://machine.domain/full/path/to/file\n"
"\n"
, stdout);