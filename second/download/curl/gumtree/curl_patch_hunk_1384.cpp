 " FTP\n"
 "\n"
 "  Upload all data on stdin to a specified ftp site:\n"
 "\n"
 "        curl -t ftp://ftp.upload.com/myfile\n"
 "\n"
-);
- puts(
 "  Upload data from a specified file, login with user and password:\n"
 "\n"
+);
+ puts(
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload a local file to the remote site, and use the local file name remote\n"
 "  too:\n"
 " \n"
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/\n"
