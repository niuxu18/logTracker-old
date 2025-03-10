"\n"
"              content_type   The Content-Type of the  requested  document,  if\n"
"                             there was any. (Added in 7.9.5)\n"
"\n"
, stdout);
 fputs(
"              num_connects   Number  of new connects made in the recent trans-\n"
"                             fer. (Added in 7.12.3)\n"
"\n"
"              num_redirects  Number of redirects that  were  followed  in  the\n"
"                             request. (Added in 7.12.3)\n"
"\n"
"       If this option is used several times, the last one will be used.\n"
"\n"
"       -x/--proxy <proxyhost[:port]>\n"
"              Use  specified  HTTP proxy. If the port number is not specified,\n"
"              it is assumed at port 1080.\n"
"\n"
, stdout);
 fputs(
"              This option overrides existing environment variables  that  sets\n"
"              proxy  to  use.  If  there's  an  environment variable setting a\n"
"              proxy, you can set proxy to \"\" to override it.\n"
"\n"
"              Note that all operations that are performed over  a  HTTP  proxy\n"
"              will  transparently  be converted to HTTP. It means that certain\n"
"              protocol specific operations might not be available. This is not\n"
, stdout);
 fputs(
"              the  case  if you can tunnel through the proxy, as done with the\n"
"              -p/--proxytunnel option.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -X/--request <command>\n"
"              (HTTP) Specifies a custom request to use when communicating with\n"
"              the HTTP server.  The specified request will be used instead  of\n"
"              the  standard  GET.  Read the HTTP 1.1 specification for details\n"
"              and explanations.\n"
"\n"
, stdout);
 fputs(
"              (FTP) Specifies a custom FTP command to use instead of LIST when\n"
"              doing file lists with ftp.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -y/--speed-time <time>\n"
"              If a download is slower than speed-limit bytes per second during\n"
"              a speed-time period, the download gets aborted. If speed-time is\n"
"              used, the default speed-limit will be 1 unless set with -y.\n"
"\n"
, stdout);
 fputs(
"              This option controls transfers and thus  will  not  affect  slow\n"
"              connects  etc.  If this is a concern for you, try the --connect-\n"
"              timeout option.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -Y/--speed-limit <speed>\n"
"              If a download is slower than this given speed, in bytes per sec-\n"
"              ond, for speed-time seconds it gets aborted. speed-time  is  set\n"
, stdout);
 fputs(
"              with -Y and is 30 if not set.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -z/--time-cond <date expression>\n"
"              (HTTP) Request a file that has  been  modified  later  than  the\n"
"              given  time  and date, or one that has been modified before that\n"
"              time. The date expression can be all sorts of date strings or if\n"
"              it  doesn't  match  any  internal ones, it tries to get the time\n"
, stdout);
 fputs(
"              from a given file name  instead!  See  the  curl_getdate(3)  man\n"
"              pages for date expression details.\n"
"\n"
"              Start the date expression with a dash (-) to make it request for\n"
"              a document that is older than the given date/time, default is  a\n"
"              document that is newer than the specified date/time.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --max-redirs <num>\n"
, stdout);
 fputs(
"              Set  maximum  number  of  redirection-followings   allowed.   If\n"
"              -L/--location  is  used, this option can be used to prevent curl\n"
"              from following redirections \"in absurdum\".\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -0/--http1.0\n"
"              (HTTP)  Forces curl to issue its requests using HTTP 1.0 instead\n"
"              of using its internally preferred: HTTP 1.1.\n"
"\n"
"       -1/--tlsv1\n"
, stdout);
 fputs(
"              (HTTPS) Forces curl to use TSL version 1 when negotiating with a\n"
"              remote TLS server.\n"
"\n"
"       -2/--sslv2\n"
"              (HTTPS) Forces curl to use SSL version 2 when negotiating with a\n"
"              remote SSL server.\n"
"\n"
"       -3/--sslv3\n"
"              (HTTPS) Forces curl to use SSL version 3 when negotiating with a\n"
"              remote SSL server.\n"
"\n"
"       --3p-quote\n"
"              (FTP)  Specify  arbitrary commands to send to the source server.\n"
, stdout);
 fputs(
"              See the -Q/--quote option for details. (Added in 7.13.0)\n"
"\n"
"       --3p-url\n"
"              (FTP) Activates a FTP 3rd party transfer. Specifies  the  source\n"
"              URL  to  get a file from, while the \"normal\" URL will be used as\n"
"              target URL, the file that will be written/created.\n"
"\n"
"              Note that not all FTP server allow 3rd party  transfers.  (Added\n"
"              in 7.13.0)\n"
"\n"
"       --3p-user\n"
, stdout);
 fputs(
"              (FTP)  Specify user:password for the source URL transfer. (Added\n"
"              in 7.13.0)\n"
"\n"
"       -4/--ipv4\n"
"              If libcurl is capable of resolving an  address  to  multiple  IP\n"
"              versions  (which it is if it is ipv6-capable), this option tells\n"
"              libcurl to resolve names  to  IPv4  addresses  only.  (Added  in\n"
"              7.10.8)\n"
"\n"
"       -6/--ipv6\n"
"              If  libcurl  is  capable  of resolving an address to multiple IP\n"
, stdout);
 fputs(
"              versions (which it is if it is ipv6-capable), this option  tells\n"
"              libcurl  to  resolve  names  to  IPv6  addresses only. (Added in\n"
"              7.10.8)\n"
"\n"
"       -#/--progress-bar\n"
"              Make curl display progress information as a progress bar instead\n"
"              of the default statistics.\n"
"\n"
"              If  this option is used twice, the second will again disable the\n"
"              progress bar.\n"
"\n"
"FILES\n"
"       ~/.curlrc\n"
"              Default config file.\n"
"\n"
, stdout);
 fputs(
"ENVIRONMENT\n"
"       http_proxy [protocol://]<host>[:port]\n"
"              Sets proxy server to use for HTTP.\n"
"\n"
"       HTTPS_PROXY [protocol://]<host>[:port]\n"
"              Sets proxy server to use for HTTPS.\n"
"\n"
"       FTP_PROXY [protocol://]<host>[:port]\n"
"              Sets proxy server to use for FTP.\n"
"\n"
"       GOPHER_PROXY [protocol://]<host>[:port]\n"
"              Sets proxy server to use for GOPHER.\n"
"       ALL_PROXY [protocol://]<host>[:port]\n"
, stdout);
 fputs(
"              Sets proxy server to use if no protocol-specific proxy is set.\n"
"\n"
"       NO_PROXY <comma-separated list of hosts>\n"
"              list of host names that shouldn't go through any proxy.  If  set\n"
"              to a asterisk\n"
"\n"
"EXIT CODES\n"
"       There  exists  a bunch of different error codes and their corresponding\n"
"       error messages that may appear during bad conditions. At  the  time  of\n"
"       this writing, the exit codes are:\n"
"\n"
, stdout);
 fputs(
"       1      Unsupported protocol. This build of curl has no support for this\n"
"              protocol.\n"
"\n"
"       2      Failed to initialize.\n"
"\n"
"       3      URL malformat. The syntax was not correct.\n"
"\n"
"       4      URL user malformatted. The user-part of the URL syntax  was  not\n"
"              correct.\n"
"\n"
"       5      Couldn't  resolve  proxy.  The  given  proxy  host  could not be\n"
"              resolved.\n"
"\n"
"       6      Couldn't resolve host. The given remote host was not resolved.\n"
"\n"
, stdout);
 fputs(
"       7      Failed to connect to host.\n"
"\n"
"       8      FTP weird server reply.  The  server  sent  data  curl  couldn't\n"
"              parse.\n"
"\n"
"       9      FTP access denied. The server denied login.\n"
"\n"
"       10     FTP  user/password  incorrect.  Either  one  or  both  were  not\n"
"              accepted by the server.\n"
"\n"
"       11     FTP weird PASS reply. Curl couldn't parse the reply sent to  the\n"
"              PASS request.\n"
"\n"
, stdout);
 fputs(
"       12     FTP  weird USER reply. Curl couldn't parse the reply sent to the\n"
"              USER request.\n"
"\n"
"       13     FTP weird PASV reply, Curl couldn't parse the reply sent to  the\n"
"              PASV request.\n"
"\n"
"       14     FTP  weird  227  format.  Curl  couldn't  parse the 227-line the\n"
"              server sent.\n"
"\n"
"       15     FTP can't get host. Couldn't resolve the host IP we got  in  the\n"
"              227-line.\n"
"\n"
, stdout);
 fputs(
"       16     FTP  can't reconnect. Couldn't connect to the host we got in the\n"
"              227-line.\n"
"\n"
"       17     FTP couldn't set binary.  Couldn't  change  transfer  method  to\n"
"              binary.\n"
"\n"
"       18     Partial file. Only a part of the file was transferred.\n"
"\n"
"       19     FTP  couldn't download/access the given file, the RETR (or simi-\n"
"              lar) command failed.\n"
"\n"
"       20     FTP write error. The transfer was reported bad by the server.\n"
"\n"
, stdout);
 fputs(
"       21     FTP quote error. A quote command returned error from the server.\n"
"       22     HTTP  page  not  retrieved.  The  requested url was not found or\n"
"              returned another error with the HTTP error  code  being  400  or\n"
"              above. This return code only appears if -f/--fail is used.\n"
"\n"
"       23     Write  error.  Curl couldn't write data to a local filesystem or\n"
"              similar.\n"
"\n"
"       24     Malformed user. User name badly specified.\n"
"\n"
, stdout);
 fputs(
"       25     FTP couldn't STOR file. The server denied  the  STOR  operation,\n"
"              used for FTP uploading.\n"
"\n"
"       26     Read error. Various reading problems.\n"
"\n"
"       27     Out of memory. A memory allocation request failed.\n"
"\n"
"       28     Operation  timeout.  The  specified  time-out period was reached\n"
"              according to the conditions.\n"
"\n"
"       29     FTP couldn't set ASCII. The server returned an unknown reply.\n"
"\n"
, stdout);
 fputs(
"       30     FTP PORT failed. The PORT command failed. Not  all  FTP  servers\n"
"              support  the  PORT  command,  try  doing  a  transfer using PASV\n"
"              instead!\n"
"\n"
"       31     FTP couldn't use REST. The REST command failed. This command  is\n"
"              used for resumed FTP transfers.\n"
"\n"
"       32     FTP  couldn't  use SIZE. The SIZE command failed. The command is\n"
"              an extension to the original FTP spec RFC 959.\n"
"\n"
, stdout);
 fputs(
"       33     HTTP range error. The range \"command\" didn't work.\n"
"\n"
"       34     HTTP post error. Internal post-request generation error.\n"
"\n"
"       35     SSL connect error. The SSL handshaking failed.\n"
"\n"
"       36     FTP bad download resume. Couldn't continue  an  earlier  aborted\n"
"              download.\n"
"\n"
"       37     FILE couldn't read file. Failed to open the file. Permissions?\n"
"\n"
"       38     LDAP cannot bind. LDAP bind operation failed.\n"
"\n"
