 "\n"
 "KERBEROS4 FTP TRANSFER\n"
 "\n"
 "  Curl supports kerberos4 for FTP transfers. You need the kerberos package\n"
 "  installed and used at curl build time for it to be used.\n"
 "\n"
-);
- puts(
+, stdout);
+ fputs(
 "  First, get the krb-ticket the normal way, like with the kauth tool. Then use\n"
 "  curl in way similar to:\n"
 "\n"
 "        curl --krb4 private ftp://krb4site.com -u username:fakepwd\n"
 "\n"
 "  There's no use for a password on the -u switch, but a blank one will make\n"
 "  curl ask for one and you already entered the real password to kauth.\n"
 "\n"
 "TELNET\n"
 "\n"
 "  The curl telnet support is basic and very easy to use. Curl passes all data\n"
 "  passed to it on stdin to the remote server. Connect to a remote telnet\n"
-);
- puts(
+, stdout);
+ fputs(
 "  server using a command line similar to:\n"
 "\n"
 "        curl telnet://remote.server.com\n"
 "\n"
 "  And enter the data to pass to the server on stdin. The result will be sent\n"
 "  to stdout or to the file you specify with -o.\n"
