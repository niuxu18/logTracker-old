"              was built to support HTTP/2. This option overrides --http1.1 and\n"
, stdout);
 fputs(
"              -0, --http1.0 and --http2-prior-knowledge. Added in 7.33.0.\n"
"\n"
"       --ignore-content-length\n"
"              (FTP  HTTP)  For HTTP, Ignore the Content-Length header. This is\n"
"              particularly useful for servers running Apache 1.x,  which  will\n"
"              report  incorrect  Content-Length  for files larger than 2 giga-\n"
"              bytes.\n"
"\n"
"              For FTP (since 7.46.0), skip the RETR command to figure out  the\n"
"              size before downloading a file.\n"
"\n"
, stdout);
 fputs(
"       -i, --include\n"
"              Include  the HTTP-header in the output. The HTTP-header includes\n"
"              things like server-name, date of the document, HTTP-version  and\n"
"              more...\n"
"\n"
"              See also -v, --verbose.\n"
"\n"
"       -k, --insecure\n"
"              (TLS)  This  option explicitly allows curl to perform \"insecure\"\n"
"              SSL connections and transfers. All SSL connections are attempted\n"
"              to  be  made secure by using the CA certificate bundle installed\n"
, stdout);
 fputs(
"              by default. This makes  all  connections  considered  \"insecure\"\n"
"              fail unless -k, --insecure is used.\n"
"\n"
"              See this online resource for further details:\n"
"               https://curl.haxx.se/docs/sslcerts.html\n"
"\n"
"       --interface <name>\n"
"\n"
"              Perform  an operation using a specified interface. You can enter\n"
"              interface name, IP address or host name. An example  could  look\n"
"              like:\n"
"\n"
, stdout);
 fputs(
"               curl --interface eth0:1 https://www.example.com/\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"              See also --dns-interface.\n"
"\n"
"       -4, --ipv4\n"
"              This  option tells curl to resolve names to IPv4 addresses only,\n"
"              and not for example try IPv6.\n"
"\n"
"              See also  --http1.1  and  --http2.  This  option  overrides  -6,\n"
"              --ipv6.\n"
"\n"
"       -6, --ipv6\n"
, stdout);
 fputs(
"              This  option tells curl to resolve names to IPv6 addresses only,\n"
"              and not for example try IPv4.\n"
"\n"
"              See also  --http1.1  and  --http2.  This  option  overrides  -6,\n"
"              --ipv6.\n"
"\n"
"       -j, --junk-session-cookies\n"
"              (HTTP) When curl is told to read cookies from a given file, this\n"
"              option will make it discard all  \"session  cookies\".  This  will\n"
"              basically  have  the same effect as if a new session is started.\n"
, stdout);
 fputs(
"              Typical browsers always discard  session  cookies  when  they're\n"
"              closed down.\n"
"\n"
"              See also -b, --cookie and -c, --cookie-jar.\n"
"\n"
"       --keepalive-time <seconds>\n"
"              This  option  sets  the  time  a connection needs to remain idle\n"
"              before sending keepalive probes and the time between  individual\n"
"              keepalive probes. It is currently effective on operating systems\n"
, stdout);
 fputs(
"              offering  the  TCP_KEEPIDLE  and  TCP_KEEPINTVL  socket  options\n"
"              (meaning  Linux, recent AIX, HP-UX and more). This option has no\n"
"              effect if --no-keepalive is used.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"              If unspecified, the option defaults to 60 seconds.\n"
"\n"
"              Added in 7.18.0.\n"
"\n"
"       --key-type <type>\n"
"              (TLS)  Private key file type. Specify which type your --key pro-\n"
, stdout);
 fputs(
"              vided private key is. DER, PEM, and ENG are  supported.  If  not\n"
"              specified, PEM is assumed.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --key <key>\n"
"              (TLS SSH) Private key file name. Allows you to provide your pri-\n"
"              vate key in this separate file. For SSH, if not specified,  curl\n"
"              tries the following candidates in order:\n"
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
"              If this option is used several times, the last one will be used.\n"
, stdout);
 fputs(
"              --krb requires that the underlying libcurl was built to  support\n"
"              Kerberos.\n"
"\n"
"       --libcurl <file>\n"
"              Append  this  option  to any ordinary curl command line, and you\n"
"              will get a libcurl-using C source code written to the file  that\n"
"              does the equivalent of what your command-line operation does!\n"
"\n"
"              If  this  option is used several times, the last given file name\n"
"              will be used.\n"
"\n"
, stdout);
 fputs(
"              Added in 7.16.1.\n"
"\n"
"       --limit-rate <speed>\n"
"              Specify the maximum transfer rate you want curl  to  use  -  for\n"
"              both downloads and uploads. This feature is useful if you have a\n"
"              limited pipe and you'd like your transfer not to use your entire\n"
"              bandwidth. To make it slower than it otherwise would be.\n"
"\n"
"              The  given speed is measured in bytes/second, unless a suffix is\n"
, stdout);
 fputs(
"              appended.  Appending 'k' or 'K' will count the number  as  kilo-\n"
"              bytes,  'm'  or M' makes it megabytes, while 'g' or 'G' makes it\n"
"              gigabytes. Examples: 200K, 3m and 1G.\n"
"\n"
"              If you also use the -Y, --speed-limit option, that  option  will\n"
"              take precedence and might cripple the rate-limiting slightly, to\n"
"              help keeping the speed-limit logic working.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -l, --list-only\n"
"              (FTP POP3) (FTP) When listing  an  FTP  directory,  this  switch\n"
"              forces  a  name-only view. This is especially useful if the user\n"
"              wants to machine-parse the contents of an  FTP  directory  since\n"
"              the normal directory view doesn't use a standard look or format.\n"
"              When used like this, the option causes a NLST command to be sent\n"
, stdout);
 fputs(
"              to the server instead of LIST.\n"
"\n"
"              Note:  Some  FTP  servers  list  only files in their response to\n"
"              NLST; they do not include sub-directories and symbolic links.\n"
"\n"
"              (POP3) When retrieving a specific email from POP3,  this  switch\n"
"              forces  a  LIST command to be performed instead of RETR. This is\n"
"              particularly useful if the user wants to see if a specific  mes-\n"
, stdout);
 fputs(
"              sage id exists on the server and what size it is.\n"
"\n"
"              Note:  When combined with -X, --request, this option can be used\n"
"              to send an UIDL command instead, so the user may use the email's\n"
"              unique  identifier  rather  than  it's  message  id  to make the\n"
"              request.\n"
"\n"
"              Added in 7.21.5.\n"
"\n"
"       --local-port <num/range>\n"
"              Set a preferred single number or range (FROM-TO) of  local  port\n"
, stdout);
 fputs(
"              numbers to use for the connection(s).  Note that port numbers by\n"
"              nature are a scarce resource that will be busy at times so  set-\n"
"              ting  this range to something too narrow might cause unnecessary\n"
"              connection setup failures.\n"
"\n"
"              Added in 7.15.2.\n"
"\n"
"       --location-trusted\n"
"              (HTTP) Like -L, --location, but will allow sending  the  name  +\n"
"              password to all hosts that the site may redirect to. This may or\n"
, stdout);
 fputs(
"              may not introduce a security breach if the site redirects you to\n"
"              a  site  to which you'll send your authentication info (which is\n"
"              plaintext in the case of HTTP Basic authentication).\n"
"\n"
"              See also -u, --user.\n"
"\n"
"       -L, --location\n"
"              (HTTP) If the server reports that the requested page  has  moved\n"
"              to a different location (indicated with a Location: header and a\n"
, stdout);
 fputs(
"              3XX response code), this option will make curl redo the  request\n"
"              on  the  new  place.  If used together with -i, --include or -I,\n"
"              --head, headers from all requested pages  will  be  shown.  When\n"
"              authentication  is  used, curl only sends its credentials to the\n"
"              initial host. If a redirect takes curl to a different  host,  it\n"
"              won't  be  able to intercept the user+password. See also --loca-\n"
, stdout);
 fputs(
"              tion-trusted on how to change this. You can limit the amount  of\n"
"              redirects to follow by using the --max-redirs option.\n"
"\n"
"              When  curl follows a redirect and the request is not a plain GET\n"
"              (for example POST or PUT), it will do the following request with\n"
"              a GET if the HTTP response was 301, 302, or 303. If the response\n"
"              code was any other 3xx code, curl  will  re-send  the  following\n"
, stdout);
 fputs(
"              request using the same unmodified method.\n"
"\n"
"              You  can  tell  curl to not change the non-GET request method to\n"
"              GET after a 30x response by  using  the  dedicated  options  for\n"
"              that: --post301, --post302 and --post303.\n"
"\n"
"       --login-options <options>\n"
"              (IMAP  POP3 SMTP) Specify the login options to use during server\n"
"              authentication.\n"
"\n"
"              You can use the  login  options  to  specify  protocol  specific\n"
, stdout);
 fputs(
"              options  that may be used during authentication. At present only\n"
"              IMAP, POP3 and SMTP support login options. For more  information\n"
"              about  the  login options please see RFC 2384, RFC 5092 and IETF\n"
"              draft draft-earhart-url-smtp-00.txt\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"              Added in 7.34.0.\n"
"\n"
"       --mail-auth <address>\n"
, stdout);
 fputs(
"              (SMTP) Specify a single address. This will be  used  to  specify\n"
"              the  authentication  address  (identity)  of a submitted message\n"
"              that is being relayed to another server.\n"
"\n"
"              See also --mail-rcpt and --mail-from. Added in 7.25.0.\n"
"\n"
"       --mail-from <address>\n"
"              (SMTP) Specify a single address that the given mail  should  get\n"
"              sent from.\n"
"\n"
"              See also --mail-rcpt and --mail-auth. Added in 7.20.0.\n"
"\n"
, stdout);
 fputs(
"       --mail-rcpt <address>\n"
"              (SMTP) Specify a single address, user name or mailing list name.\n"
"              Repeat this option several times to send to multiple recipients.\n"
"              When performing a mail transfer, the recipient should specify  a\n"
"              valid email address to send the mail to.\n"
"\n"
"              When  performing  an  address  verification  (VRFY command), the\n"
"              recipient should be specified as the user name or user name  and\n"
, stdout);
 fputs(
"              domain (as per Section 3.5 of RFC5321). (Added in 7.34.0)\n"
"\n"
"              When performing a mailing list expand (EXPN command), the recip-\n"
"              ient should be specified using the mailing list  name,  such  as\n"
"              \"Friends\" or \"London-Office\".  (Added in 7.34.0)\n"
"\n"
"              Added in 7.20.0.\n"
"\n"
"       -M, --manual\n"
"              Manual. Display the huge help text.\n"
"\n"
"       --max-filesize <bytes>\n"
, stdout);
 fputs(
"              Specify  the  maximum  size (in bytes) of a file to download. If\n"
"              the file requested is larger than this value, the transfer  will\n"
"              not start and curl will return with exit code 63.\n"
"\n"
"              NOTE:  The  file size is not always known prior to download, and\n"
"              for such files this option has no effect even if the file trans-\n"
"              fer  ends  up  being larger than this given limit. This concerns\n"
, stdout);
 fputs(
"              both FTP and HTTP transfers.\n"
"\n"
"              See also --limit-rate.\n"
"\n"
"       --max-redirs <num>\n"
"              (HTTP) Set maximum  number  of  redirection-followings  allowed.\n"
"              When  -L,  --location is used, is used to prevent curl from fol-\n"
"              lowing redirections \"in absurdum\". By default, the limit is  set\n"
"              to 50 redirections. Set this option to -1 to make it unlimited.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -m, --max-time <time>\n"
"              Maximum  time  in  seconds that you allow the whole operation to\n"
"              take.  This is useful for preventing your batch jobs from  hang-\n"
"              ing  for  hours due to slow networks or links going down.  Since\n"
"              7.32.0, this option accepts decimal values, but the actual time-\n"
, stdout);
 fputs(
"              out will decrease in accuracy as the specified timeout increases\n"
"              in decimal precision.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"              See also --connect-timeout.\n"
"\n"
"       --metalink\n"
"              This option can tell curl to parse and process a  given  URI  as\n"
"              Metalink  file  (both  version 3 and 4 (RFC 5854) are supported)\n"
"              and make use of the mirrors listed within for failover if  there\n"
, stdout);
 fputs(
"              are  errors (such as the file or server not being available). It\n"
"              will also verify the hash of the file after  the  download  com-\n"
"              pletes.  The Metalink file itself is downloaded and processed in\n"
"              memory and not stored in the local file system.\n"
"\n"
"              Example to use a remote Metalink file:\n"
"\n"
"               curl --metalink http://www.example.com/example.metalink\n"
"\n"
