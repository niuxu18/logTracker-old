 "  Get the first 100 bytes of a document:\n"
 "\n"
 "        curl -r 0-99 http://www.get.this/\n"
 "\n"
 "  Get the last 500 bytes of a document:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -r -500 http://www.get.this/\n"
 "\n"
+, stdout);
+ fputs(
 "  Curl also supports simple ranges for FTP files as well. Then you can only\n"
 "  specify start and stop position.\n"
 "\n"
 "  Get the first 100 bytes of a document using FTP:\n"
 "\n"
 "        curl -r 0-99 ftp://www.get.this/README  \n"
