 "  Get the definition of curl from a dictionary:\n"
 "\n"
 "        curl dict://dict.org/m:curl\n"
 "\n"
 "  Fetch two documents at once:\n"
 "\n"
-, stdout);
- fputs(
 "        curl ftp://cool.haxx.se/ http://www.weirdserver.com:8000/\n"
 "\n"
 "  Get a file off an FTPS server:\n"
 "\n"
 "        curl ftps://files.are.secure.com/secrets.txt\n"
 "\n"
+, stdout);
+ fputs(
 "  or use the more appropriate FTPS way to get the same file:\n"
 "\n"
 "        curl --ftp-ssl ftp://files.are.secure.com/secrets.txt\n"
 "\n"
 "  Get a file from an SSH server using SFTP:\n"
 "\n"
 "        curl -u username sftp://shell.example.com/etc/issue\n"
 "\n"
 "  Get a file from an SSH server using SCP using a private key to authenticate:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -u username: --key ~/.ssh/id_dsa --pubkey ~/.ssh/id_dsa.pub \\\n"
 "        	scp://shell.example.com/~/personal.txt\n"
 "\n"
 "  Get the main page from an IPv6 web server:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -g \"http://[2001:1890:1112:1::20]/\"\n"
 "\n"
 "DOWNLOAD TO A FILE\n"
 "\n"
 "  Get a web page and store in a local file:\n"
 "\n"
 "        curl -o thatpage.html http://www.netscape.com/\n"
 "\n"
 "  Get a web page and store in a local file, make the local file get the name\n"
 "  of the remote document (if no file name part is specified in the URL, this\n"
-, stdout);
- fputs(
 "  will fail):\n"
 "\n"
 "        curl -O http://www.netscape.com/index.html\n"
 "\n"
 "  Fetch two files and store them with their remote names:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -O www.haxx.se/index.html -O curl.haxx.se/download.html\n"
 "\n"
 "USING PASSWORDS\n"
 "\n"
 " FTP\n"
 "\n"
