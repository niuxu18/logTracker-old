fputs(
" standards for those, but it can still be made to work with many of them. You\n"
" can also use both HTTP and SOCKS proxies to transfer files to and from FTP\n"
" servers.\n"
"\n"
" Get an ftp file using an HTTP proxy named my-proxy that uses port 888:\n"
"\n"
"        curl -x my-proxy:888 ftp://ftp.leachsite.com/README\n"
"\n"
" Get a file from a HTTP server that requires user and password, using the\n"
" same proxy as above:\n"
"\n"
"        curl -u user:passwd -x my-proxy:888 http://www.get.this/\n"
"\n"
, stdout);