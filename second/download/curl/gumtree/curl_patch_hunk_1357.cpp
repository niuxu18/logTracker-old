 " Continue uploading a document(*1):\n"
 "\n"
 "        curl -c -T file ftp://ftp.server.com/path/file\n"
 "\n"
 " Continue downloading a document from a web server(*2):\n"
 "\n"
+);
+ puts(
 "        curl -c -o file http://www.server.com/\n"
 "\n"
 " (*1) = This requires that the ftp server supports the non-standard command\n"
 "        SIZE. If it doesn't, curl will say so.\n"
 "\n"
 " (*2) = This requires that the wb server supports at least HTTP/1.1. If it\n"
