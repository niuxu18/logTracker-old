fputs(
"       --proto-default <protocol>\n"
"              Tells curl to use protocol for any URL missing a scheme name.\n"
"\n"
"              Example:\n"
"\n"
"               curl --proto-default https ftp.mozilla.org\n"
"\n"
"              An unknown or unsupported  protocol  causes  error  CURLE_UNSUP-\n"
"              PORTED_PROTOCOL (1).\n"
"\n"
"              This option does not change the default proxy protocol (http).\n"
"\n"
"              Without  this  option curl would make a guess based on the host,\n"
, stdout);