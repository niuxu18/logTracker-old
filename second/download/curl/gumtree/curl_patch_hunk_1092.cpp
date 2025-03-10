 "\n"
 "        curl -r -500 http://www.get.this/\n"
 "\n"
 "  Curl also supports simple ranges for FTP files as well. Then you can only\n"
 "  specify start and stop position.\n"
 "\n"
-, stdout);
- fputs(
 "  Get the first 100 bytes of a document using FTP:\n"
 "\n"
 "        curl -r 0-99 ftp://www.get.this/README\n"
 "\n"
 "UPLOADING\n"
 "\n"
 " FTP / FTPS / SFTP / SCP\n"
 "\n"
 "  Upload all data on stdin to a specified server:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -T - ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload data from a specified file, login with user and password:\n"
 "\n"
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload a local file to the remote site, and use the local file name at the remote\n"
 "  site too:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/\n"
 "\n"
 "  Upload a local file to get appended to the remote file:\n"
 "\n"
 "        curl -T localfile -a ftp://ftp.upload.com/remotefile\n"
 "\n"
+, stdout);
+ fputs(
 "  Curl also supports ftp upload through a proxy, but only if the proxy is\n"
 "  configured to allow that kind of tunneling. If it does, you can run curl in\n"
 "  a fashion similar to:\n"
 "\n"
 "        curl --proxytunnel -x proxy:port -T localfile ftp.upload.com\n"
 "\n"
 "SMB / SMBS\n"
 "\n"
 "        curl -T file.txt -u \"domain\\username:passwd\" \n"
-, stdout);
- fputs(
 "         smb://server.example.com/share/\n"
 "\n"
 " HTTP\n"
 "\n"
 "  Upload all data on stdin to a specified HTTP site:\n"
 "\n"
 "        curl -T - http://www.upload.com/myfile\n"
 "\n"
+, stdout);
+ fputs(
 "  Note that the HTTP server must have been configured to accept PUT before\n"
 "  this can be done successfully.\n"
 "\n"
 "  For other ways to do HTTP data upload, see the POST section below.\n"
 "\n"
 "VERBOSE / DEBUG\n"
 "\n"
 "  If curl fails where it isn't supposed to, if the servers don't let you in,\n"
 "  if you can't understand the responses: use the -v flag to get verbose\n"
-, stdout);
- fputs(
 "  fetching. Curl will output lots of info and what it sends and receives in\n"
 "  order to let the user see all client-server interaction (but it won't show\n"
+, stdout);
+ fputs(
 "  you the actual data).\n"
 "\n"
 "        curl -v ftp://ftp.upload.com/\n"
 "\n"
 "  To get even more details and information on what curl does, try using the\n"
 "  --trace or --trace-ascii options with a given file name to log to, like\n"
