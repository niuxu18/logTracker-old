fputs(
"        curl -B \"ldap://ldap.frontec.se/o=frontec??sub?mail=*sth.frontec.se\"\n"
"\n"
"  If I want the same info in HTML format, I can get it by not using the -B\n"
"  (enforce ASCII) flag.\n"
"\n"
"ENVIRONMENT VARIABLES\n"
"\n"
"  Curl reads and understands the following environment variables:\n"
"\n"
"        http_proxy, HTTPS_PROXY, FTP_PROXY\n"
"\n"
"  They should be set for protocol-specific proxies. General proxy should be\n"
"  set with\n"
"        \n"
"        ALL_PROXY\n"
"\n"
, stdout);