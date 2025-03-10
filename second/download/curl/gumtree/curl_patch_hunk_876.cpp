 "        curl --ftp-ssl ftp://files.are.secure.com/secrets.txt\n"
 "\n"
 "  Get a file from an SSH server using SFTP:\n"
 "\n"
 "        curl -u username sftp://example.com/etc/issue\n"
 "\n"
+, stdout);
+ fputs(
 "  Get a file from an SSH server using SCP using a private key\n"
 "  (not password-protected) to authenticate:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -u username: --key ~/.ssh/id_rsa \\\n"
 "             scp://example.com/~/file.txt\n"
 "\n"
 "  Get a file from an SSH server using SCP using a private key\n"
 "  (password-protected) to authenticate:\n"
 "\n"
 "        curl -u username: --key ~/.ssh/id_rsa --pass private_key_password \\\n"
 "             scp://example.com/~/file.txt\n"
 "\n"
 "  Get the main page from an IPv6 web server:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl \"http://[2001:1890:1112:1::20]/\"\n"
 "\n"
+"  Get a file from an SMB server:\n"
+"\n"
+"        curl -u \"domain\\username:passwd\" smb://server.example.com/share/file.txt\n"
+"\n"
 "DOWNLOAD TO A FILE\n"
 "\n"
 "  Get a web page and store in a local file with a specific name:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -o thatpage.html http://www.netscape.com/\n"
 "\n"
 "  Get a web page and store in a local file, make the local file get the name\n"
 "  of the remote document (if no file name part is specified in the URL, this\n"
 "  will fail):\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -O http://www.netscape.com/index.html\n"
 "\n"
 "  Fetch two files and store them with their remote names:\n"
 "\n"
 "        curl -O www.haxx.se/index.html -O curl.haxx.se/download.html\n"
 "\n"
 "USING PASSWORDS\n"
 "\n"
 " FTP\n"
 "\n"
 "   To ftp files using name+passwd, include them in the URL like:\n"
 "\n"
-, stdout);
- fputs(
 "        curl ftp://name:passwd@machine.domain:port/full/path/to/file\n"
 "\n"
 "   or specify them with the -u flag like\n"
 "\n"
 "        curl -u name:passwd ftp://machine.domain:port/full/path/to/file\n"
 "\n"
 " FTPS\n"
 "\n"
+, stdout);
+ fputs(
 "   It is just like for FTP, but you may also want to specify and use\n"
 "   SSL-specific options for certificates etc.\n"
 "\n"
 "   Note that using FTPS:// as prefix is the \"implicit\" way as described in the\n"
 "   standards while the recommended \"explicit\" way is done by using FTP:// and\n"
 "   the --ftp-ssl option.\n"
 "\n"
-, stdout);
- fputs(
 " SFTP / SCP\n"
 "\n"
 "   This is similar to FTP, but you can use the --key option to specify a\n"
 "   private key to use instead of a password. Note that the private key may\n"
+, stdout);
+ fputs(
 "   itself be protected by a password that is unrelated to the login password\n"
 "   of the remote system; this password is specified using the --pass option.\n"
 "   Typically, curl will automatically extract the public key from the private\n"
 "   key file, but in cases where curl does not have the proper library support,\n"
-, stdout);
- fputs(
 "   a matching public key file must be specified using the --pubkey option.\n"
 "\n"
 " HTTP\n"
 "\n"
 "   Curl also supports user and password in HTTP URLs, thus you can pick a file\n"
 "   like:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl http://name:passwd@machine.domain/full/path/to/file\n"
 "\n"
 "   or specify user and password separately like in\n"
 "\n"
 "        curl -u name:passwd http://machine.domain/full/path/to/file\n"
 "\n"
 "   HTTP offers many different methods of authentication and curl supports\n"
-, stdout);
- fputs(
 "   several: Basic, Digest, NTLM and Negotiate (SPNEGO). Without telling which\n"
 "   method to use, curl defaults to Basic. You can also ask curl to pick the\n"
 "   most secure ones out of the ones that the server accepts for the given URL,\n"
+, stdout);
+ fputs(
 "   by using --anyauth.\n"
 "\n"
 "   NOTE! According to the URL specification, HTTP URLs can not contain a user\n"
 "   and password, so that style will not work when using curl via a proxy, even\n"
 "   though curl allows it at other times. When using a proxy, you _must_ use\n"
-, stdout);
- fputs(
 "   the -u style for user and password.\n"
 "\n"
 " HTTPS\n"
 "\n"
 "   Probably most commonly used with private certificates, as explained below.\n"
 "\n"
 "PROXY\n"
 "\n"
 " curl supports both HTTP and SOCKS proxy servers, with optional authentication.\n"
+, stdout);
+ fputs(
 " It does not have special support for FTP proxy servers since there are no\n"
 " standards for those, but it can still be made to work with many of them. You\n"
 " can also use both HTTP and SOCKS proxies to transfer files to and from FTP\n"
 " servers.\n"
 "\n"
-, stdout);
- fputs(
 " Get an ftp file using an HTTP proxy named my-proxy that uses port 888:\n"
 "\n"
 "        curl -x my-proxy:888 ftp://ftp.leachsite.com/README\n"
 "\n"
 " Get a file from an HTTP server that requires user and password, using the\n"
 " same proxy as above:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -u user:passwd -x my-proxy:888 http://www.get.this/\n"
 "\n"
 " Some proxies require special authentication. Specify by using -U as above:\n"
 "\n"
 "        curl -U user:passwd -x my-proxy:888 http://www.get.this/\n"
 "\n"
-, stdout);
- fputs(
 " A comma-separated list of hosts and domains which do not use the proxy can\n"
 " be specified as:\n"
 "\n"
 "        curl --noproxy localhost,get.this -x my-proxy:888 http://www.get.this/\n"
 "\n"
 " If the proxy is specified with --proxy1.0 instead of --proxy or -x, then\n"
+, stdout);
+ fputs(
 " curl will use HTTP/1.0 instead of HTTP/1.1 for any CONNECT attempts.\n"
 "\n"
 " curl also supports SOCKS4 and SOCKS5 proxies with --socks4 and --socks5.\n"
 "\n"
 " See also the environment variables Curl supports that offer further proxy\n"
 " control.\n"
 "\n"
-, stdout);
- fputs(
 " Most FTP proxy servers are set up to appear as a normal FTP server from the\n"
 " client's perspective, with special commands to select the remote FTP server.\n"
 " curl supports the -u, -Q and --ftp-account options that can be used to\n"
+, stdout);
+ fputs(
 " set up transfers through many FTP proxies. For example, a file can be\n"
 " uploaded to a remote FTP server using a Blue Coat FTP proxy with the\n"
 " options:\n"
 "\n"
 "   curl -u \"Remote-FTP-Username@remote.ftp.server Proxy-Username:Remote-Pass\" \\\n"
-, stdout);
- fputs(
 "    --ftp-account Proxy-Password --upload-file local-file \\\n"
 "    ftp://my-ftp.proxy.server:21/remote/upload/path/\n"
 "\n"
 " See the manual for your FTP proxy to determine the form it expects to set up\n"
 " transfers, and curl's -v option to see exactly what curl is sending.\n"
 "\n"
+, stdout);
+ fputs(
 "RANGES\n"
 "\n"
 "  HTTP 1.1 introduced byte-ranges. Using this, a client can request\n"
 "  to get only one or more subparts of a specified document. Curl supports\n"
 "  this with the -r flag.\n"
 "\n"
 "  Get the first 100 bytes of a document:\n"
 "\n"
-, stdout);
- fputs(
 "        curl -r 0-99 http://www.get.this/\n"
 "\n"
 "  Get the last 500 bytes of a document:\n"
 "\n"
 "        curl -r -500 http://www.get.this/\n"
 "\n"
 "  Curl also supports simple ranges for FTP files as well. Then you can only\n"
 "  specify start and stop position.\n"
 "\n"
+, stdout);
+ fputs(
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
 "        curl -T - ftp://ftp.upload.com/myfile\n"
 "\n"
-, stdout);
- fputs(
 "  Upload data from a specified file, login with user and password:\n"
 "\n"
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/myfile\n"
 "\n"
 "  Upload a local file to the remote site, and use the local file name at the remote\n"
 "  site too:\n"
 "\n"
+, stdout);
+ fputs(
 "        curl -T uploadfile -u user:passwd ftp://ftp.upload.com/\n"
 "\n"
 "  Upload a local file to get appended to the remote file:\n"
 "\n"
 "        curl -T localfile -a ftp://ftp.upload.com/remotefile\n"
 "\n"
 "  Curl also supports ftp upload through a proxy, but only if the proxy is\n"
-, stdout);
- fputs(
 "  configured to allow that kind of tunneling. If it does, you can run curl in\n"
 "  a fashion similar to:\n"
 "\n"
 "        curl --proxytunnel -x proxy:port -T localfile ftp.upload.com\n"
 "\n"
+"SMB / SMBS\n"
+"\n"
+"        curl -T file.txt -u \"domain\\username:passwd\" \n"
+, stdout);
+ fputs(
+"         smb://server.example.com/share/\n"
+"\n"
 " HTTP\n"
 "\n"
 "  Upload all data on stdin to a specified HTTP site:\n"
 "\n"
 "        curl -T - http://www.upload.com/myfile\n"
 "\n"
