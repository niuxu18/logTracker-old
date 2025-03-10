"       curl [options] [URL...]\n"
"\n"
"DESCRIPTION\n"
"       curl  is  a tool to transfer data from or to a server, using one of the\n"
, stdout);
 fputs(
"       supported protocols (HTTP, HTTPS, FTP, FTPS, TFTP, DICT,  TELNET,  LDAP\n"
"       or FILE).  The command is designed to work without user interaction.\n"
"\n"
"       curl offers a busload of useful tricks like proxy support, user authen-\n"
"       tication, ftp upload, HTTP post, SSL connections, cookies, file  trans-\n"
"       fer resume and more. As you will see below, the amount of features will\n"
"       make your head spin!\n"
"\n"
, stdout);
 fputs(
"       curl is powered by  libcurl  for  all  transfer-related  features.  See\n"
"       libcurl(3) for details.\n"
"\n"
"URL\n"
"       The  URL  syntax is protocol dependent. You'll find a detailed descrip-\n"
"       tion in RFC 3986.\n"
"\n"
"       You can specify multiple URLs or parts of URLs  by  writing  part  sets\n"
"       within braces as in:\n"
"\n"
"        http://site.{one,two,three}.com\n"
"\n"
"       or you can get sequences of alphanumeric series by using [] as in:\n"
"\n"
