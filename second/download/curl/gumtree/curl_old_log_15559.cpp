fputs(
"  They should be set for protocol-specific proxies. General proxy should be\n"
"  set with\n"
"\n"
"        ALL_PROXY\n"
"\n"
"  A comma-separated list of host names that shouldn't go through any proxy is\n"
"  set in (only an asterisk, '*' matches all hosts)\n"
"\n"
"        NO_PROXY\n"
"\n"
"  If the host name matches one of these strings, or the host is within the\n"
"  domain of one of these strings, transactions with that node will not be\n"
"  proxied.\n"
"\n"
"\n"
"  The usage of the -x/--proxy flag overrides the environment variables.\n"
"\n"
"NETRC\n"
"\n"
, stdout);