"       tax (optionally with one of the numbers omitted). FTP  use  depends  on\n"
"       the extended FTP command SIZE.\n"
"\n"
"       If this option is used several times, the last one will be used.\n"
"\n"
"       -R, --remote-time\n"
"              When  used,  this  will  make  libcurl attempt to figure out the\n"
"              timestamp of the remote file, and if that is available make  the\n"
"              local file get that same timestamp.\n"
"\n"
"       --random-file <file>\n"
, stdout);
 fputs(
"              (SSL) Specify the path name to file containing what will be con-\n"
"              sidered as random data. The data is  used  to  seed  the  random\n"
"              engine for SSL connections.  See also the --egd-file option.\n"
"\n"
"       --raw  When  used, it disables all internal HTTP decoding of content or\n"
"              transfer encodings and instead makes them passed  on  unaltered,\n"
"              raw. (Added in 7.16.2)\n"
"\n"
"       --remote-name-all\n"
, stdout);
 fputs(
"              This  option changes the default action for all given URLs to be\n"
"              dealt with as if -O, --remote-name were used for each one. So if\n"
