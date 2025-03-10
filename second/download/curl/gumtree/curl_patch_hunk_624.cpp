 "              nsspem.so) is available then PEM files may be loaded.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --capath <CA certificate directory>\n"
 "              (SSL) Tells curl to use the specified certificate  directory  to\n"
-"              verify  the peer. The certificates must be in PEM format, and if\n"
+"              verify  the  peer.  Multiple paths can be provided by separating\n"
 , stdout);
  fputs(
-"              curl is built against OpenSSL, the directory must have been pro-\n"
-"              cessed  using  the c_rehash utility supplied with OpenSSL. Using\n"
-"              --capath can allow OpenSSL-powered curl to make  SSL-connections\n"
-"              much  more  efficiently than using --cacert if the --cacert file\n"
-"              contains many CA certificates.\n"
+"              them with \":\" (e.g.  \"path1:path2:path3\"). The certificates must\n"
+"              be  in  PEM  format,  and  if curl is built against OpenSSL, the\n"
+"              directory must have been processed using  the  c_rehash  utility\n"
+"              supplied  with OpenSSL. Using --capath can allow OpenSSL-powered\n"
+"              curl to make SSL-connections much more  efficiently  than  using\n"
+"              --cacert if the --cacert file contains many CA certificates.\n"
 "\n"
-"              If this option is used several times, the last one will be used.\n"
-"\n"
-"       -f, --fail\n"
 , stdout);
  fputs(
+"              If this option is set, the default capath value will be ignored,\n"
+"              and if it is used several times, the last one will be used.\n"
+"\n"
+"       -f, --fail\n"
 "              (HTTP) Fail silently (no output at all) on server  errors.  This\n"
 "              is  mostly done to better enable scripts etc to better deal with\n"
 "              failed attempts. In normal cases when a  HTTP  server  fails  to\n"
 "              deliver  a  document,  it  returns  an  HTML document stating so\n"
+, stdout);
+ fputs(
 "              (which often also describes why and more). This flag  will  pre-\n"
 "              vent curl from outputting that and return error 22.\n"
 "\n"
-, stdout);
- fputs(
 "              This  method is not fail-safe and there are occasions where non-\n"
 "              successful response codes will  slip  through,  especially  when\n"
 "              authentication is involved (response codes 401 and 407).\n"
 "\n"
 "       -F, --form <name=content>\n"
 "              (HTTP)  This  lets curl emulate a filled-in form in which a user\n"
-"              has pressed the submit button. This causes  curl  to  POST  data\n"
-"              using  the  Content-Type  multipart/form-data  according  to RFC\n"
 , stdout);
  fputs(
+"              has pressed the submit button. This causes  curl  to  POST  data\n"
+"              using  the  Content-Type  multipart/form-data  according  to RFC\n"
 "              2388. This enables uploading of binary files etc. To  force  the\n"
 "              'content'  part  to  be  a  file, prefix the file name with an @\n"
 "              sign. To just get the content part from a file, prefix the  file\n"
 "              name  with  the symbol <. The difference between @ and < is then\n"
-"              that @ makes a file get attached in the post as a  file  upload,\n"
-"              while  the  <  makes  a text field and just get the contents for\n"
 , stdout);
  fputs(
+"              that @ makes a file get attached in the post as a  file  upload,\n"
+"              while  the  <  makes  a text field and just get the contents for\n"
 "              that text field from a file.\n"
 "\n"
 "              Example, to send your password file to the server, where  'pass-\n"
 "              word' is the name of the form-field to which /etc/passwd will be\n"
 "              the input:\n"
 "\n"
 "              curl -F password=@/etc/passwd www.mypasswords.com\n"
 "\n"
+, stdout);
+ fputs(
 "              To read content from stdin instead of a file, use - as the file-\n"
 "              name. This goes for both @ and < constructs.\n"
 "\n"
-, stdout);
- fputs(
 "              You  can  also  tell  curl  what  Content-Type  to  use by using\n"
 "              'type=', in a manner similar to:\n"
 "\n"
 "              curl -F \"web=@index.html;type=text/html\" url.com\n"
 "\n"
 "              or\n"
 "\n"
 "              curl -F \"name=daniel;type=text/foo\" url.com\n"
 "\n"
 "              You can also explicitly change the name field of a  file  upload\n"
+, stdout);
+ fputs(
 "              part by setting filename=, like this:\n"
 "\n"
 "              curl -F \"file=@localfile;filename=nameinpost\" url.com\n"
 "\n"
-, stdout);
- fputs(
 "              See further examples and details in the MANUAL.\n"
 "\n"
 "              This option can be used multiple times.\n"
 "\n"
 "       --ftp-account [data]\n"
 "              (FTP) When an FTP server asks for \"account data\" after user name\n"
 "              and password has been provided, this data is sent off using  the\n"
 "              ACCT command. (Added in 7.13.0)\n"
 "\n"
+, stdout);
+ fputs(
 "              If  this option is used twice, the second will override the pre-\n"
 "              vious use.\n"
 "\n"
 "       --ftp-alternative-to-user <command>\n"
-, stdout);
- fputs(
 "              (FTP) If authenticating with the USER and PASS  commands  fails,\n"
 "              send  this  command.   When  connecting  to  Tumbleweed's Secure\n"
 "              Transport server over FTPS using  a  client  certificate,  using\n"
 "              \"SITE  AUTH\"  will tell the server to retrieve the username from\n"
+, stdout);
+ fputs(
 "              the certificate. (Added in 7.15.5)\n"
 "\n"
 "       --ftp-create-dirs\n"
 "              (FTP/SFTP) When an FTP or SFTP URL/operation uses  a  path  that\n"
-, stdout);
- fputs(
 "              doesn't  currently exist on the server, the standard behavior of\n"
 "              curl is to fail. Using this option, curl will instead attempt to\n"
 "              create missing directories.\n"
 "\n"
 "       --ftp-method [method]\n"
 "              (FTP)  Control  what method curl should use to reach a file on a\n"
+, stdout);
+ fputs(
 "              FTP(S) server. The method argument should be one of the  follow-\n"
 "              ing alternatives:\n"
 "\n"
 "              multicwd\n"
-, stdout);
- fputs(
 "                     curl  does  a  single CWD operation for each path part in\n"
 "                     the given URL. For deep hierarchies this means very  many\n"
 "                     commands.  This  is  how RFC 1738 says it should be done.\n"
 "                     This is the default but the slowest behavior.\n"
 "\n"
-"              nocwd  curl does no CWD at all. curl will do  SIZE,  RETR,  STOR\n"
-"                     etc and give a full path to the server for all these com-\n"
 , stdout);
  fputs(
+"              nocwd  curl does no CWD at all. curl will do  SIZE,  RETR,  STOR\n"
+"                     etc and give a full path to the server for all these com-\n"
 "                     mands. This is the fastest behavior.\n"
 "\n"
 "              singlecwd\n"
 "                     curl does one CWD with the full target directory and then\n"
 "                     operates  on  the  file  \"normally\" (like in the multicwd\n"
 "                     case). This is somewhat  more  standards  compliant  than\n"
+, stdout);
+ fputs(
 "                     'nocwd' but without the full penalty of 'multicwd'.\n"
 "       (Added in 7.15.1)\n"
 "\n"
 "       --ftp-pasv\n"
-, stdout);
- fputs(
 "              (FTP)  Use  passive mode for the data connection. Passive is the\n"
 "              internal default behavior, but using this option can be used  to\n"
 "              override a previous -P/-ftp-port option. (Added in 7.11.0)\n"
 "\n"
 "              If  this option is used several times, the following occurrences\n"
+, stdout);
+ fputs(
 "              make no difference. Undoing an  enforced  passive  really  isn't\n"
 "              doable  but you must then instead enforce the correct -P, --ftp-\n"
 "              port again.\n"
 "\n"
-, stdout);
- fputs(
 "              Passive mode means that curl will try the EPSV command first and\n"
 "              then PASV, unless --disable-epsv is used.\n"
 "\n"
 "       --ftp-skip-pasv-ip\n"
 "              (FTP) Tell curl to not use the IP address the server suggests in\n"
+, stdout);
+ fputs(
 "              its response to curl's PASV command when curl connects the  data\n"
 "              connection.  Instead  curl  will  re-use  the same IP address it\n"
 "              already uses for the control connection. (Added in 7.14.2)\n"
 "\n"
-, stdout);
- fputs(
 "              This option has no effect if PORT, EPRT or EPSV is used  instead\n"
 "              of PASV.\n"
 "\n"
 "       --ftp-pret\n"
 "              (FTP)  Tell  curl to send a PRET command before PASV (and EPSV).\n"
+, stdout);
+ fputs(
 "              Certain FTP servers, mainly drftpd,  require  this  non-standard\n"
 "              command  for  directory  listings as well as up and downloads in\n"
 "              PASV mode.  (Added in 7.20.x)\n"
 "\n"
 "       --ftp-ssl-ccc\n"
-, stdout);
- fputs(
 "              (FTP) Use CCC (Clear Command Channel)  Shuts  down  the  SSL/TLS\n"
 "              layer after authenticating. The rest of the control channel com-\n"
 "              munication will be unencrypted. This allows NAT routers to  fol-\n"
+, stdout);
+ fputs(
 "              low the FTP transaction. The default mode is passive. See --ftp-\n"
 "              ssl-ccc-mode for other modes.  (Added in 7.16.1)\n"
 "\n"
 "       --ftp-ssl-ccc-mode [active/passive]\n"
-, stdout);
- fputs(
 "              (FTP) Use CCC (Clear Command Channel) Sets  the  CCC  mode.  The\n"
 "              passive  mode  will  not initiate the shutdown, but instead wait\n"
 "              for the server to do it, and will not reply to the shutdown from\n"
+, stdout);
+ fputs(
 "              the server. The active mode initiates the shutdown and waits for\n"
 "              a reply from the server.  (Added in 7.16.2)\n"
 "\n"
 "       --ftp-ssl-control\n"
 "              (FTP) Require SSL/TLS for the FTP  login,  clear  for  transfer.\n"
-, stdout);
- fputs(
 "              Allows  secure  authentication, but non-encrypted data transfers\n"
 "              for efficiency.  Fails the transfer if the server  doesn't  sup-\n"
 "              port SSL/TLS.  (Added in 7.16.0) that can still be used but will\n"
+, stdout);
+ fputs(
 "              be removed in a future version.\n"
 "\n"
 "       --form-string <name=string>\n"
 "              (HTTP) Similar to --form except that the value  string  for  the\n"
 "              named  parameter  is used literally. Leading '@' and '<' charac-\n"
-, stdout);
- fputs(
 "              ters, and the ';type=' string in the value have no special mean-\n"
 "              ing. Use this in preference to --form if there's any possibility\n"
 "              that the string value may accidentally trigger the  '@'  or  '<'\n"
+, stdout);
+ fputs(
 "              features of --form.\n"
 "\n"
 "       -g, --globoff\n"
 "              This option switches off the \"URL globbing parser\". When you set\n"
 "              this option, you can specify URLs that contain the letters  {}[]\n"
-, stdout);
- fputs(
 "              without  having them being interpreted by curl itself. Note that\n"
 "              these letters are not normal legal URL contents but they  should\n"
 "              be encoded according to the URI standard.\n"
 "\n"
 "       -G, --get\n"
+, stdout);
+ fputs(
 "              When  used,  this  option  will make all data specified with -d,\n"
 "              --data or --data-binary to be used in a HTTP GET request instead\n"
 "              of  the POST request that otherwise would be used. The data will\n"
-, stdout);
- fputs(
 "              be appended to the URL with a '?' separator.\n"
 "\n"
 "              If used in combination with -I, the POST data  will  instead  be\n"
 "              appended to the URL with a HEAD request.\n"
 "\n"
+, stdout);
+ fputs(
 "              If  this option is used several times, the following occurrences\n"
 "              make no difference. This is because undoing a GET  doesn't  make\n"
 "              sense,  but  you  should  then  instead  enforce the alternative\n"
 "              method you prefer.\n"
 "\n"
 "       -H, --header <header>\n"
-, stdout);
- fputs(
 "              (HTTP) Extra header to use when getting  a  web  page.  You  may\n"
 "              specify any number of extra headers. Note that if you should add\n"
+, stdout);
+ fputs(
 "              a custom header that has the same name as one  of  the  internal\n"
 "              ones  curl  would  use,  your externally set header will be used\n"
 "              instead of the internal one. This allows you to make even trick-\n"
 "              ier  stuff  than  curl would normally do. You should not replace\n"
-, stdout);
- fputs(
 "              internally set  headers  without  knowing  perfectly  well  what\n"
 "              you're  doing. Remove an internal header by giving a replacement\n"
+, stdout);
+ fputs(
 "              without content on the right  side  of  the  colon,  as  in:  -H\n"
 "              \"Host:\".  If  you  send the custom header with no-value then its\n"
 "              header must be terminated with a semicolon, such as  -H  \"X-Cus-\n"
 "              tom-Header;\" to send \"X-Custom-Header:\".\n"
 "\n"
-, stdout);
- fputs(
 "              curl  will  make  sure  that each header you add/replace is sent\n"
 "              with the proper end-of-line marker, you should thus not add that\n"
+, stdout);
+ fputs(
 "              as a part of the header content: do not add newlines or carriage\n"
 "              returns, they will only mess things up for you.\n"
 "\n"
 "              See also the -A, --user-agent and -e, --referer options.\n"
 "\n"
 "              This option can be used  multiple  times  to  add/replace/remove\n"
 "              multiple headers.\n"
 "\n"
-, stdout);
- fputs(
 "       --hostpubmd5 <md5>\n"
 "              Pass  a  string  containing  32  hexadecimal  digits. The string\n"
+, stdout);
+ fputs(
 "              should be the 128 bit MD5 checksum of the remote  host's  public\n"
 "              key,  curl  will  refuse the connection with the host unless the\n"
 "              md5sums match. This option is only for SCP and  SFTP  transfers.\n"
 "              (Added in 7.17.1)\n"
 "\n"
 "       --ignore-content-length\n"
 "              (HTTP)  Ignore  the  Content-Length header. This is particularly\n"
+"              useful for servers running Apache 1.x, which will report  incor-\n"
 , stdout);
  fputs(
-"              useful for servers running Apache 1.x, which will report  incor-\n"
 "              rect Content-Length for files larger than 2 gigabytes.\n"
 "\n"
 "       -i, --include\n"
 "              (HTTP)  Include  the  HTTP-header in the output. The HTTP-header\n"
 "              includes things like server-name, date of  the  document,  HTTP-\n"
 "              version and more...\n"
 "\n"
 "       -I, --head\n"
 "              (HTTP/FTP/FILE) Fetch the HTTP-header only! HTTP-servers feature\n"
+"              the command HEAD which this uses to get nothing but  the  header\n"
 , stdout);
  fputs(
-"              the command HEAD which this uses to get nothing but  the  header\n"
 "              of  a  document.  When used on a FTP or FILE file, curl displays\n"
 "              the file size and last modification time only.\n"
 "\n"
 "       --interface <name>\n"
 "              Perform an operation using a specified interface. You can  enter\n"
 "              interface  name,  IP address or host name. An example could look\n"
 "              like:\n"
 "\n"
 "               curl --interface eth0:1 http://www.netscape.com/\n"
 "\n"
-, stdout);
- fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+, stdout);
+ fputs(
 "       -j, --junk-session-cookies\n"
 "              (HTTP) When curl is told to read cookies from a given file, this\n"
 "              option  will  make  it  discard all \"session cookies\". This will\n"
 "              basically have the same effect as if a new session  is  started.\n"
 "              Typical  browsers  always  discard  session cookies when they're\n"
 "              closed down.\n"
 "\n"
 "       -J, --remote-header-name\n"
+"              (HTTP) This option tells the -O, --remote-name option to use the\n"
 , stdout);
  fputs(
-"              (HTTP) This option tells the -O, --remote-name option to use the\n"
 "              server-specified   Content-Disposition   filename   instead   of\n"
 "              extracting a filename from the URL.\n"
 "\n"
 "       -k, --insecure\n"
 "              (SSL) This option explicitly allows curl to  perform  \"insecure\"\n"
 "              SSL connections and transfers. All SSL connections are attempted\n"
 "              to be made secure by using the CA certificate  bundle  installed\n"
+"              by  default.  This  makes  all connections considered \"insecure\"\n"
 , stdout);
  fputs(
-"              by  default.  This  makes  all connections considered \"insecure\"\n"
 "              fail unless -k, --insecure is used.\n"
 "\n"
 "              See    this    online    resource    for    further     details:\n"
 "              http://curl.haxx.se/docs/sslcerts.html\n"
 "\n"
 "       -K, --config <config file>\n"
 "              Specify  which config file to read curl arguments from. The con-\n"
 "              fig file is a text file in which command line arguments  can  be\n"
+"              written  which  then will be used as if they were written on the\n"
 , stdout);
  fputs(
-"              written  which  then will be used as if they were written on the\n"
 "              actual command line. Options and their parameters must be speci-\n"
 "              fied  on  the  same  config  file line, separated by whitespace,\n"
 "              colon, the equals sign or any combination thereof (however,  the\n"
 "              preferred  separator is the equals sign). If the parameter is to\n"
 "              contain  whitespace,  the  parameter  must  be  enclosed  within\n"
+"              quotes. Within double quotes, the following escape sequences are\n"
 , stdout);
  fputs(
-"              quotes. Within double quotes, the following escape sequences are\n"
 "              available: \\\\, \\\", \\t, \\n, \\r and \\v. A backslash preceding  any\n"
 "              other letter is ignored. If the first column of a config line is\n"
 "              a '#' character, the rest of the line will be treated as a  com-\n"
 "              ment.  Only  write  one  option  per physical line in the config\n"
 "              file.\n"
 "\n"
 "              Specify the filename to -K, --config as '-' to  make  curl  read\n"
-, stdout);
- fputs(
 "              the file from stdin.\n"
 "\n"
+, stdout);
+ fputs(
 "              Note  that  to  be able to specify a URL in the config file, you\n"
 "              need to specify it using the --url option,  and  not  by  simply\n"
 "              writing  the  URL  on its own line. So, it could look similar to\n"
 "              this:\n"
 "\n"
 "              url = \"http://curl.haxx.se/docs/\"\n"
