"              connection.  Instead  curl  will  re-use  the same IP address it\n"
"              already uses for the control connection. (Added in 7.14.2)\n"
"\n"
"              This option has no effect if PORT, EPRT or EPSV is used  instead\n"
"              of PASV.\n"
"\n"
"       --ftp-pret\n"
"              (FTP)  Tell  curl to send a PRET command before PASV (and EPSV).\n"
, stdout);
 fputs(
"              Certain FTP servers, mainly drftpd,  require  this  non-standard\n"
"              command  for  directory  listings as well as up and downloads in\n"
"              PASV mode.  (Added in 7.20.x)\n"
"\n"
"       --ssl  (FTP, POP3, IMAP, SMTP) Try to use SSL/TLS for  the  connection.\n"
"              Reverts to a non-secure connection if the server doesn't support\n"
"              SSL/TLS.  See also --ftp-ssl-control and --ssl-reqd for  differ-\n"
, stdout);
 fputs(
"              ent levels of encryption required. (Added in 7.20.0)\n"
"\n"
"              This  option  was  formerly known as --ftp-ssl (Added in 7.11.0)\n"
"              and that can still be used but will be removed in a future  ver-\n"
"              sion.\n"
"\n"
"       --ftp-ssl-control\n"
"              (FTP)  Require  SSL/TLS  for  the FTP login, clear for transfer.\n"
"              Allows secure authentication, but non-encrypted  data  transfers\n"
, stdout);
 fputs(
"              for  efficiency.   Fails the transfer if the server doesn't sup-\n"
"              port SSL/TLS.  (Added in 7.16.0)\n"
"\n"
"       --ssl-reqd\n"
"              (FTP, POP3, IMAP, SMTP)  Require  SSL/TLS  for  the  connection.\n"
"              Terminates the connection if the server doesn't support SSL/TLS.\n"
"              (Added in 7.20.0)\n"
"\n"
"              This option was  formerly  known  as  --ftp-ssl-reqd  (added  in\n"
"              7.15.5)  and  that  can  still  be used but will be removed in a\n"
, stdout);
 fputs(
"              future version.\n"
"\n"
"       --ftp-ssl-ccc\n"
"              (FTP) Use CCC (Clear Command Channel)  Shuts  down  the  SSL/TLS\n"
"              layer after authenticating. The rest of the control channel com-\n"
"              munication will be unencrypted. This allows NAT routers to  fol-\n"
"              low the FTP transaction. The default mode is passive. See --ftp-\n"
"              ssl-ccc-mode for other modes.  (Added in 7.16.1)\n"
"\n"
"       --ftp-ssl-ccc-mode [active/passive]\n"
, stdout);
 fputs(
"              (FTP) Use CCC (Clear Command Channel) Sets  the  CCC  mode.  The\n"
"              passive  mode  will  not initiate the shutdown, but instead wait\n"
"              for the server to do it, and will not reply to the shutdown from\n"
"              the server. The active mode initiates the shutdown and waits for\n"
"              a reply from the server.  (Added in 7.16.2)\n"
"\n"
"       -F/--form <name=content>\n"
"              (HTTP) This lets curl emulate a filled-in form in which  a  user\n"
, stdout);
 fputs(
"              has  pressed  the  submit  button. This causes curl to POST data\n"
"              using the Content-Type multipart/form-data according to RFC2388.\n"
"              This  enables  uploading of binary files etc. To force the 'con-\n"
"              tent' part to be a file, prefix the file name with an @ sign. To\n"
"              just get the content part from a file, prefix the file name with\n"
"              the symbol <. The difference between @ and  <  is  then  that  @\n"
, stdout);
 fputs(
"              makes  a  file  get attached in the post as a file upload, while\n"
"              the < makes a text field and just get the contents for that text\n"
"              field from a file.\n"
"\n"
"              Example,  to send your password file to the server, where 'pass-\n"
"              word' is the name of the form-field to which /etc/passwd will be\n"
"              the input:\n"
"\n"
"              curl -F password=@/etc/passwd www.mypasswords.com\n"
"\n"
, stdout);
 fputs(
"              To  read  the file's content from stdin instead of a file, use -\n"
"              where the file name should've been. This goes for both @  and  <\n"
"              constructs.\n"
"\n"
"              You  can  also  tell  curl  what  Content-Type  to  use by using\n"
"              'type=', in a manner similar to:\n"
"\n"
"              curl -F \"web=@index.html;type=text/html\" url.com\n"
"\n"
"              or\n"
"\n"
"              curl -F \"name=daniel;type=text/foo\" url.com\n"
"\n"
, stdout);
 fputs(
"              You can also explicitly change the name field of an file  upload\n"
"              part by setting filename=, like this:\n"
"\n"
"              curl -F \"file=@localfile;filename=nameinpost\" url.com\n"
"\n"
"              See further examples and details in the MANUAL.\n"
"\n"
"              This option can be used multiple times.\n"
"\n"
"       --form-string <name=string>\n"
"              (HTTP)  Similar  to  --form except that the value string for the\n"
, stdout);
 fputs(
"              named parameter is used literally. Leading '@' and  '<'  charac-\n"
"              ters, and the ';type=' string in the value have no special mean-\n"
"              ing. Use this in preference to --form if there's any possibility\n"
"              that  the  string  value may accidentally trigger the '@' or '<'\n"
"              features of --form.\n"
"\n"
"       -g/--globoff\n"
"              This option switches off the \"URL globbing parser\". When you set\n"
, stdout);
 fputs(
"              this  option, you can specify URLs that contain the letters {}[]\n"
"              without having them being interpreted by curl itself. Note  that\n"
"              these  letters are not normal legal URL contents but they should\n"
"              be encoded according to the URI standard.\n"
"\n"
"       -G/--get\n"
"              When used,  this  option  will  make  all  data  specified  with\n"
"              -d/--data  or  --data-binary  to  be  used in a HTTP GET request\n"
, stdout);
 fputs(
"              instead of the POST request that otherwise would  be  used.  The\n"
"              data will be appended to the URL with a '?' separator.\n"
"\n"
"              If  used  in  combination with -I, the POST data will instead be\n"
"              appended to the URL with a HEAD request.\n"
"\n"
"              If this option is used several times, the following  occurrences\n"
"              make  no  difference. This is because undoing a GET doesn't make\n"
, stdout);
 fputs(
"              sense, but you  should  then  instead  enforce  the  alternative\n"
"              method you prefer.\n"
"\n"
"       -h/--help\n"
"              Usage help.\n"
"\n"
"       -H/--header <header>\n"
"              (HTTP)  Extra  header  to  use  when getting a web page. You may\n"
"              specify any number of extra headers. Note that if you should add\n"
"              a  custom  header  that has the same name as one of the internal\n"
, stdout);
 fputs(
"              ones curl would use, your externally set  header  will  be  used\n"
"              instead of the internal one. This allows you to make even trick-\n"
"              ier stuff than curl would normally do. You  should  not  replace\n"
"              internally  set  headers  without  knowing  perfectly  well what\n"
"              you're doing. Remove an internal header by giving a  replacement\n"
"              without  content  on  the  right  side  of  the colon, as in: -H\n"
"              \"Host:\".\n"
"\n"
, stdout);
 fputs(
"              curl will make sure that each header  you  add/replace  is  sent\n"
"              with the proper end-of-line marker, you should thus not add that\n"
"              as a part of the header content: do not add newlines or carriage\n"
"              returns, they will only mess things up for you.\n"
"\n"
"              See also the -A/--user-agent and -e/--referer options.\n"
"\n"
"              This  option  can  be  used multiple times to add/replace/remove\n"
"              multiple headers.\n"
"\n"
, stdout);
 fputs(
"       --hostpubmd5 <md5>\n"
"              Pass a string  containing  32  hexadecimal  digits.  The  string\n"
"              should  be  the 128 bit MD5 checksum of the remote host's public\n"
"              key, curl will refuse the connection with the  host  unless  the\n"
"              md5sums  match.  This option is only for SCP and SFTP transfers.\n"
"              (Added in 7.17.1)\n"
"\n"
"       --ignore-content-length\n"
"              (HTTP) Ignore the Content-Length header.  This  is  particularly\n"
, stdout);
 fputs(
"              useful  for servers running Apache 1.x, which will report incor-\n"
"              rect Content-Length for files larger than 2 gigabytes.\n"
"\n"
"       -i/--include\n"
"              (HTTP) Include the HTTP-header in the  output.  The  HTTP-header\n"
"              includes  things  like  server-name, date of the document, HTTP-\n"
"              version and more...\n"
"\n"
"       --interface <name>\n"
"              Perform an operation using a specified interface. You can  enter\n"
, stdout);
 fputs(
"              interface  name,  IP address or host name. An example could look\n"
"              like:\n"
"\n"
"               curl --interface eth0:1 http://www.netscape.com/\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -I/--head\n"
"              (HTTP/FTP/FILE) Fetch the HTTP-header only! HTTP-servers feature\n"
"              the  command  HEAD which this uses to get nothing but the header\n"
"              of a document. When used on a FTP or FILE  file,  curl  displays\n"
, stdout);
 fputs(
"              the file size and last modification time only.\n"
"\n"
"       -j/--junk-session-cookies\n"
"              (HTTP) When curl is told to read cookies from a given file, this\n"
"              option will make it discard all  \"session  cookies\".  This  will\n"
"              basically  have  the same effect as if a new session is started.\n"
"              Typical browsers always discard  session  cookies  when  they're\n"
"              closed down.\n"
"\n"
"       -J/--remote-header-name\n"
, stdout);
 fputs(
"              (HTTP)  This option tells the -O/--remote-name option to use the\n"
"              server-specified   Content-Disposition   filename   instead   of\n"
"              extracting a filename from the URL.\n"
"\n"
"       -k/--insecure\n"
"              (SSL)  This  option explicitly allows curl to perform \"insecure\"\n"
"              SSL connections and transfers. All SSL connections are attempted\n"
"              to  be  made secure by using the CA certificate bundle installed\n"
, stdout);
 fputs(
"              by default. This makes  all  connections  considered  \"insecure\"\n"
"              fail unless -k/--insecure is used.\n"
"\n"
"              See     this    online    resource    for    further    details:\n"
"              http://curl.haxx.se/docs/sslcerts.html\n"
"\n"
"       --keepalive-time <seconds>\n"
"              This option sets the time a  connection  needs  to  remain  idle\n"
"              before  sending keepalive probes and the time between individual\n"
, stdout);
 fputs(
"              keepalive probes. It is currently effective on operating systems\n"
"              offering  the  TCP_KEEPIDLE  and  TCP_KEEPINTVL  socket  options\n"
"              (meaning Linux, recent AIX, HP-UX and more). This option has  no\n"
"              effect if --no-keepalive is used. (Added in 7.18.0)\n"
"\n"
"              If  this option is used multiple times, the last occurrence sets\n"
"              the amount.\n"
"       --key <key>\n"
, stdout);
 fputs(
"              (SSL/SSH) Private key file name. Allows you to provide your pri-\n"
"              vate key in this separate file.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --key-type <type>\n"
"              (SSL)  Private key file type. Specify which type your --key pro-\n"
"              vided private key is. DER, PEM, and ENG are  supported.  If  not\n"
"              specified, PEM is assumed.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --krb <level>\n"
"              (FTP)  Enable Kerberos authentication and use. The level must be\n"
"              entered and should be one of 'clear', 'safe', 'confidential', or\n"
"              'private'.  Should  you  use  a  level that is not one of these,\n"
"              'private' will instead be used.\n"
"\n"
"              This option requires a library built with  kerberos4  or  GSSAPI\n"
, stdout);
 fputs(
"              (GSS-Negotiate)  support. This is not very common. Use -V/--ver-\n"
"              sion to see if your curl supports it.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -K/--config <config file>\n"
"              Specify which config file to read curl arguments from. The  con-\n"
"              fig  file  is a text file in which command line arguments can be\n"
"              written which then will be used as if they were written  on  the\n"
, stdout);
 fputs(
"              actual command line. Options and their parameters must be speci-\n"
"              fied on the same config  file  line,  separated  by  whitespace,\n"
"              colon,  the equals sign or any combination thereof (however, the\n"
"              preferred separator is the equals sign). If the parameter is  to\n"
"              contain  whitespace,  the  parameter  must  be  enclosed  within\n"
"              quotes. Within double quotes, the following escape sequences are\n"
, stdout);
 fputs(
"              available:  \\\\, \\\", \\t, \\n, \\r and \\v. A backslash preceding any\n"
"              other letter is ignored. If the first column of a config line is\n"
"              a  '#' character, the rest of the line will be treated as a com-\n"
"              ment. Only write one option per  physical  line  in  the  config\n"
"              file.\n"
"\n"
"              Specify the filename to -K/--config as '-' to make curl read the\n"
"              file from stdin.\n"
"\n"
, stdout);
 fputs(
"              Note that to be able to specify a URL in the  config  file,  you\n"
"              need  to  specify  it  using the --url option, and not by simply\n"
"              writing the URL on its own line. So, it could  look  similar  to\n"
"              this:\n"
"\n"
"              url = \"http://curl.haxx.se/docs/\"\n"
"\n"
"              Long  option  names  can  optionally be given in the config file\n"
"              without the initial double dashes.\n"
"\n"
, stdout);
 fputs(
"              When curl is invoked, it always (unless -q is used) checks for a\n"
"              default  config  file  and  uses it if found. The default config\n"
"              file is checked for in the following places in this order:\n"
"\n"
"              1) curl tries to find the \"home dir\": It first  checks  for  the\n"
"              CURL_HOME and then the HOME environment variables. Failing that,\n"
"              it uses getpwuid() on UNIX-like systems (which returns the  home\n"
, stdout);
 fputs(
"              dir  given the current user in your system). On Windows, it then\n"
"              checks for the APPDATA variable, or as a last resort the '%USER-\n"
"              PROFILE%\\Application Data'.\n"
"\n"
"              2)  On  windows, if there is no _curlrc file in the home dir, it\n"
"              checks for one in the same dir the curl executable is placed. On\n"
"              UNIX-like  systems,  it will simply try to load .curlrc from the\n"
"              determined home dir.\n"
"\n"
, stdout);
 fputs(
"              # --- Example file ---\n"
"              # this is a comment\n"
