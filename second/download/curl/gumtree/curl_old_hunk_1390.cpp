"   or specify them with the -u flag like\n"
"\n"
"        curl -u name:passwd ftp://machine.domain:port/full/path/to/file\n"
"\n"
" HTTP\n"
"\n"
"   The HTTP URL doesn't support user and password in the URL string. Curl\n"
);
 puts(
"   does support that anyway to provide a ftp-style interface and thus you can\n"
"   pick a file like:\n"
"\n"
"        curl http://name:passwd@machine.domain/full/path/to/file\n"
"\n"
"   or specify user and password separately like in\n"
"\n"
"        curl -u name:passwd http://machine.domain/full/path/to/file\n"
"\n"
"   NOTE! Since HTTP URLs don't support user and password, you can't use that\n"
"   style when using Curl via a proxy. You _must_ use the -u style fetch\n"
"   during such circumstances.\n"
"\n"
" HTTPS\n"
"\n"
);
 puts(
"   Probably most commonly used with private certificates, as explained below.\n"
"\n"
" GOPHER\n"
"\n"
"   Curl features no password support for gopher.\n"
"\n"
