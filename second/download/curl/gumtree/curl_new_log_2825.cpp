fputs(
"       the  server’s  response  will be indeterminable, depending on different\n"
"       server’s configuration.\n"
"\n"
"       You should also be aware that many HTTP/1.1 servers do  not  have  this\n"
"       feature  enabled,  so  that  when  you  attempt  to get a range, you’ll\n"
"       instead get the whole document.\n"
"\n"
"       FTP  range  downloads  only  support  the  simple  syntax  ’start-stop’\n"
"       (optionally with one of the numbers omitted). It depends on the non-RFC\n"
"       command SIZE.\n"
"\n"
, stdout);