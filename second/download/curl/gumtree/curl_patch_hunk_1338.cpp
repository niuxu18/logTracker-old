 "PROXY\n"
 "\n"
 " Get an ftp file using a proxy named my-proxy that uses port 888:\n"
 "\n"
 "        curl -x my-proxy:888 ftp://ftp.leachsite.com/README\n"
 "\n"
+);
+ puts(
 " Get a file from a HTTP server that requires user and password, using the\n"
 " same proxy as above:\n"
 "\n"
 "        curl -u user:passwd -x my-proxy:888 http://www.get.this/\n"
 "\n"
 " Some proxies require special authentication. Specify by using -U as above:\n"
