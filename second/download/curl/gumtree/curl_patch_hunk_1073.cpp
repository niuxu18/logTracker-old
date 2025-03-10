 "              determined home dir.\n"
 "\n"
 , stdout);
  fputs(
 "              # --- Example file ---\n"
 "              # this is a comment\n"
-"              url = \"curl.haxx.se\"\n"
+"              url = \"example.com\"\n"
 "              output = \"curlhere.html\"\n"
 "              user-agent = \"superagent/1.0\"\n"
 "\n"
 "              # and fetch another URL too\n"
-"              url = \"curl.haxx.se/docs/manpage.html\"\n"
+"              url = \"example.com/docs/manpage.html\"\n"
 "              -O\n"
-"              referer = \"http://nowhereatall.com/\"\n"
+"              referer = \"http://nowhereatall.example.com/\"\n"
 "              # --- End of example file ---\n"
 "\n"
 "              This  option  can be used multiple times to load multiple config\n"
-"              files.\n"
-"\n"
 , stdout);
  fputs(
+"              files.\n"
+"\n"
 "       --keepalive-time <seconds>\n"
 "              This option sets the time a  connection  needs  to  remain  idle\n"
 "              before  sending keepalive probes and the time between individual\n"
 "              keepalive probes. It is currently effective on operating systems\n"
 "              offering  the  TCP_KEEPIDLE  and  TCP_KEEPINTVL  socket  options\n"
 "              (meaning Linux, recent AIX, HP-UX and more). This option has  no\n"
-"              effect if --no-keepalive is used. (Added in 7.18.0)\n"
-"\n"
 , stdout);
  fputs(
+"              effect if --no-keepalive is used. (Added in 7.18.0)\n"
+"\n"
 "              If this option is used several times, the last one will be used.\n"
 "              If unspecified, the option defaults to 60 seconds.\n"
 "\n"
 "       --key <key>\n"
 "              (SSL/SSH) Private key file name. Allows you to provide your pri-\n"
 "              vate  key in this separate file. For SSH, if not specified, curl\n"
 "              tries  the  following  candidates  in  order:   '~/.ssh/id_rsa',\n"
-"              '~/.ssh/id_dsa', './id_rsa', './id_dsa'.\n"
-"\n"
 , stdout);
  fputs(
+"              '~/.ssh/id_dsa', './id_rsa', './id_dsa'.\n"
+"\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --key-type <type>\n"
 "              (SSL)  Private key file type. Specify which type your --key pro-\n"
 "              vided private key is. DER, PEM, and ENG are  supported.  If  not\n"
 "              specified, PEM is assumed.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --krb <level>\n"
-"              (FTP)  Enable Kerberos authentication and use. The level must be\n"
 , stdout);
  fputs(
+"              (FTP)  Enable Kerberos authentication and use. The level must be\n"
 "              entered and should be one of 'clear', 'safe', 'confidential', or\n"
 "              'private'.  Should  you  use  a  level that is not one of these,\n"
 "              'private' will instead be used.\n"
 "\n"
 "              This option requires a library  built  with  kerberos4  support.\n"
 "              This  is  not very common. Use -V, --version to see if your curl\n"
 "              supports it.\n"
 "\n"
+, stdout);
+ fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+"       --krb4 <level>\n"
+"              (FTP) This is the former name for --krb. Do not use.\n"
+"\n"
 "       -l, --list-only\n"
-, stdout);
- fputs(
 "              (FTP) When listing an FTP directory, this switch forces a  name-\n"
 "              only  view.  This  is  especially  useful  if  the user wants to\n"
 "              machine-parse the contents of an FTP directory since the  normal\n"
+, stdout);
+ fputs(
 "              directory  view doesn't use a standard look or format. When used\n"
 "              like this, the option causes a NLST command to be  sent  to  the\n"
 "              server instead of LIST.\n"
 "\n"
-, stdout);
- fputs(
 "              Note:  Some  FTP  servers  list  only files in their response to\n"
 "              NLST; they do not include sub-directories and symbolic links.\n"
 "\n"
 "              (POP3) When retrieving a specific email from POP3,  this  switch\n"
+, stdout);
+ fputs(
 "              forces  a  LIST command to be performed instead of RETR. This is\n"
 "              particularly useful if the user wants to see if a specific  mes-\n"
 "              sage id exists on the server and what size it is.\n"
 "\n"
-, stdout);
- fputs(
 "              Note:  When  combined  with -X, --request <command>, this option\n"
 "              can be used to send an UIDL command instead, so the user may use\n"
 "              the  email's  unique  identifier  rather than it's message id to\n"
+, stdout);
+ fputs(
 "              make the request. (Added in 7.21.5)\n"
 "\n"
 "       -L, --location\n"
 "              (HTTP/HTTPS) If the server reports that the requested  page  has\n"
 "              moved to a different location (indicated with a Location: header\n"
-, stdout);
- fputs(
 "              and a 3XX response code), this option will make  curl  redo  the\n"
 "              request on the new place. If used together with -i, --include or\n"
 "              -I, --head, headers from all requested pages will be shown. When\n"
+, stdout);
+ fputs(
 "              authentication  is  used, curl only sends its credentials to the\n"
 "              initial host. If a redirect takes curl to a different  host,  it\n"
 "              won't  be  able to intercept the user+password. See also --loca-\n"
-, stdout);
- fputs(
 "              tion-trusted on how to change this. You can limit the amount  of\n"
 "              redirects to follow by using the --max-redirs option.\n"
 "\n"
 "              When  curl follows a redirect and the request is not a plain GET\n"
+, stdout);
+ fputs(
 "              (for example POST or PUT), it will do the following request with\n"
 "              a GET if the HTTP response was 301, 302, or 303. If the response\n"
 "              code was any other 3xx code, curl  will  re-send  the  following\n"
-, stdout);
- fputs(
 "              request using the same unmodified method.\n"
 "\n"
 "              You  can  tell  curl to not change the non-GET request method to\n"
 "              GET after a 30x response by  using  the  dedicated  options  for\n"
+, stdout);
+ fputs(
 "              that: --post301, --post302 and --post303.\n"
 "\n"
 "       --libcurl <file>\n"
 "              Append  this  option  to any ordinary curl command line, and you\n"
 "              will get a libcurl-using C source code written to the file  that\n"
-, stdout);
- fputs(
 "              does the equivalent of what your command-line operation does!\n"
 "\n"
 "              If  this  option is used several times, the last given file name\n"
 "              will be used. (Added in 7.16.1)\n"
 "\n"
 "       --limit-rate <speed>\n"
+, stdout);
+ fputs(
 "              Specify the maximum transfer rate you want curl  to  use  -  for\n"
 "              both downloads and uploads. This feature is useful if you have a\n"
 "              limited pipe and you'd like your transfer not to use your entire\n"
-, stdout);
- fputs(
 "              bandwidth. To make it slower than it otherwise would be.\n"
 "\n"
 "              The  given speed is measured in bytes/second, unless a suffix is\n"
 "              appended.  Appending 'k' or 'K' will count the number  as  kilo-\n"
+, stdout);
+ fputs(
 "              bytes,  'm'  or M' makes it megabytes, while 'g' or 'G' makes it\n"
 "              gigabytes. Examples: 200K, 3m and 1G.\n"
 "\n"
 "              The given rate is the average speed counted  during  the  entire\n"
-, stdout);
- fputs(
 "              transfer. It means that curl might use higher transfer speeds in\n"
 "              short bursts, but over time it uses no more than the given rate.\n"
 "              If you also use the -Y, --speed-limit option, that  option  will\n"
+, stdout);
+ fputs(
 "              take precedence and might cripple the rate-limiting slightly, to\n"
 "              help keeping the speed-limit logic working.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --local-port <num>[-num]\n"
-, stdout);
- fputs(
 "              Set a preferred number or range of local port numbers to use for\n"
 "              the  connection(s).   Note  that  port  numbers  by nature are a\n"
 "              scarce resource that will be busy at times so setting this range\n"
+, stdout);
+ fputs(
 "              to something too narrow might cause unnecessary connection setup\n"
 "              failures. (Added in 7.15.2)\n"
 "\n"
 "       --location-trusted\n"
 "              (HTTP/HTTPS) Like -L, --location, but  will  allow  sending  the\n"
-, stdout);
- fputs(
 "              name + password to all hosts that the site may redirect to. This\n"
 "              may or may not introduce a security breach if the site redirects\n"
 "              you  to  a  site  to  which you'll send your authentication info\n"
+, stdout);
+ fputs(
 "              (which is plaintext in the case of HTTP Basic authentication).\n"
 "\n"
 "       -m, --max-time <seconds>\n"
 "              Maximum time in seconds that you allow the  whole  operation  to\n"
-, stdout);
- fputs(
 "              take.   This is useful for preventing your batch jobs from hang-\n"
 "              ing for hours due to slow networks or links going  down.   Since\n"
 "              7.32.0, this option accepts decimal values, but the actual time-\n"
+, stdout);
+ fputs(
 "              out will decrease in accuracy as the specified timeout increases\n"
 "              in decimal precision.  See also the --connect-timeout option.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
-, stdout);
- fputs(
 "       --login-options <options>\n"
 "              Specify the login options to use during server authentication.\n"
 "\n"
 "              You  can  use  the  login  options  to specify protocol specific\n"
+, stdout);
+ fputs(
 "              options that may be used during authentication. At present  only\n"
 "              IMAP,  POP3 and SMTP support login options. For more information\n"
 "              about the login options please see RFC 2384, RFC 5092  and  IETF\n"
 "              draft draft-earhart-url-smtp-00.txt (Added in 7.34.0).\n"
 "\n"
-, stdout);
- fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --mail-auth <address>\n"
 "              (SMTP)  Specify  a  single address. This will be used to specify\n"
+, stdout);
+ fputs(
 "              the authentication address (identity)  of  a  submitted  message\n"
 "              that is being relayed to another server.\n"
 "\n"
 "              (Added in 7.25.0)\n"
 "\n"
 "       --mail-from <address>\n"
 "              (SMTP)  Specify  a single address that the given mail should get\n"
 "              sent from.\n"
 "\n"
-, stdout);
- fputs(
 "              (Added in 7.20.0)\n"
 "\n"
 "       --max-filesize <bytes>\n"
 "              Specify the maximum size (in bytes) of a file  to  download.  If\n"
+, stdout);
+ fputs(
 "              the  file requested is larger than this value, the transfer will\n"
 "              not start and curl will return with exit code 63.\n"
 "\n"
 "              NOTE: The file size is not always known prior to  download,  and\n"
 "              for such files this option has no effect even if the file trans-\n"
-, stdout);
- fputs(
 "              fer ends up being larger than this given  limit.  This  concerns\n"
 "              both FTP and HTTP transfers.\n"
 "\n"
 "       --mail-rcpt <address>\n"
+, stdout);
+ fputs(
 "              (SMTP) Specify a single address, user name or mailing list name.\n"
-"              When  performing a mail transfer, the recipient should specify a\n"
+"              Repeat this option several times to send to multiple recipients.\n"
+"              When performing a mail transfer, the recipient should specify  a\n"
 "              valid email address to send the mail to. (Added in 7.20.0)\n"
 "\n"
-"              When performing an  address  verification  (VRFY  command),  the\n"
+"              When  performing  an  address  verification  (VRFY command), the\n"
+"              recipient should be specified as the user name or user name  and\n"
 , stdout);
  fputs(
-"              recipient  should be specified as the user name or user name and\n"
 "              domain (as per Section 3.5 of RFC5321). (Added in 7.34.0)\n"
 "\n"
 "              When performing a mailing list expand (EXPN command), the recip-\n"
-"              ient  should  be  specified using the mailing list name, such as\n"
+"              ient should be specified using the mailing list  name,  such  as\n"
 "              \"Friends\" or \"London-Office\".  (Added in 7.34.0)\n"
 "\n"
 "       --max-redirs <num>\n"
-"              Set maximum number of  redirection-followings  allowed.  If  -L,\n"
+"              Set  maximum  number  of  redirection-followings allowed. If -L,\n"
+"              --location is used, this option can be used to prevent curl from\n"
 , stdout);
  fputs(
-"              --location is used, this option can be used to prevent curl from\n"
-"              following redirections \"in absurdum\". By default, the  limit  is\n"
-"              set  to 50 redirections. Set this option to -1 to make it limit-\n"
+"              following  redirections  \"in absurdum\". By default, the limit is\n"
+"              set to 50 redirections. Set this option to -1 to make it  limit-\n"
 "              less.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --metalink\n"
-"              This option can tell curl to parse and process a  given  URI  as\n"
+"              This  option  can  tell curl to parse and process a given URI as\n"
+"              Metalink file (both version 3 and 4 (RFC  5854)  are  supported)\n"
 , stdout);
  fputs(
-"              Metalink  file  (both  version 3 and 4 (RFC 5854) are supported)\n"
-"              and make use of the mirrors listed within for failover if  there\n"
-"              are  errors (such as the file or server not being available). It\n"
-"              will also verify the hash of the file after  the  download  com-\n"
-"              pletes.  The Metalink file itself is downloaded and processed in\n"
+"              and  make use of the mirrors listed within for failover if there\n"
+"              are errors (such as the file or server not being available).  It\n"
+"              will  also  verify  the hash of the file after the download com-\n"
+"              pletes. The Metalink file itself is downloaded and processed  in\n"
 "              memory and not stored in the local file system.\n"
 "\n"
-, stdout);
- fputs(
 "              Example to use a remote Metalink file:\n"
 "\n"
+, stdout);
+ fputs(
 "              curl --metalink http://www.example.com/example.metalink\n"
 "\n"
 "              To use a Metalink file in the local file system, use FILE proto-\n"
 "              col (file://):\n"
 "\n"
 "              curl --metalink file://example.metalink\n"
 "\n"
-"              Please  note  that if FILE protocol is disabled, there is no way\n"
-"              to use a local Metalink file at the time of this  writing.  Also\n"
+"              Please note that if FILE protocol is disabled, there is  no  way\n"
+"              to  use  a local Metalink file at the time of this writing. Also\n"
+"              note  that  if  --metalink  and  --include  are  used  together,\n"
 , stdout);
  fputs(
-"              note  that  if  --metalink  and  --include  are  used  together,\n"
-"              --include will be ignored. This is because including headers  in\n"
-"              the  response  will break Metalink parser and if the headers are\n"
+"              --include  will be ignored. This is because including headers in\n"
+"              the response will break Metalink parser and if the  headers  are\n"
 "              included in the file described in Metalink file, hash check will\n"
 "              fail.\n"
 "\n"
 "              (Added in 7.27.0, if built against the libmetalink library.)\n"
 "\n"
 "       -n, --netrc\n"
+"              Makes curl scan the .netrc  (_netrc  on  Windows)  file  in  the\n"
 , stdout);
  fputs(
-"              Makes  curl  scan  the  .netrc  (_netrc  on Windows) file in the\n"
 "              user's home directory for login name and password. This is typi-\n"
-"              cally  used for FTP on Unix. If used with HTTP, curl will enable\n"
+"              cally used for FTP on Unix. If used with HTTP, curl will  enable\n"
 "              user authentication. See netrc(5) ftp(1) for details on the file\n"
-"              format.  Curl  will  not  complain if that file doesn't have the\n"
+"              format. Curl will not complain if that  file  doesn't  have  the\n"
 "              right permissions (it should not be either world- or group-read-\n"
-, stdout);
- fputs(
-"              able).  The environment variable \"HOME\" is used to find the home\n"
+"              able). The environment variable \"HOME\" is used to find the  home\n"
 "              directory.\n"
 "\n"
-"              A quick and very simple example of how  to  setup  a  .netrc  to\n"
-"              allow  curl to FTP to the machine host.domain.com with user name\n"
+, stdout);
+ fputs(
+"              A  quick  and  very  simple  example of how to setup a .netrc to\n"
+"              allow curl to FTP to the machine host.domain.com with user  name\n"
 "              'myself' and password 'secret' should look similar to:\n"
 "\n"
 "              machine host.domain.com login myself password secret\n"
 "\n"
 "       -N, --no-buffer\n"
+"              Disables the buffering of the output stream. In normal work sit-\n"
+"              uations, curl will use a standard buffered  output  stream  that\n"
 , stdout);
  fputs(
-"              Disables the buffering of the output stream. In normal work sit-\n"
-"              uations,  curl  will  use a standard buffered output stream that\n"
 "              will have the effect that it will output the data in chunks, not\n"
-"              necessarily  exactly  when  the data arrives.  Using this option\n"
+"              necessarily exactly when the data arrives.   Using  this  option\n"
 "              will disable that buffering.\n"
 "\n"
-"              Note that this is the negated option name  documented.  You  can\n"
+"              Note  that  this  is the negated option name documented. You can\n"
 "              thus use --buffer to enforce the buffering.\n"
 "\n"
+"       --netrc-file\n"
+"              This option is similar to --netrc, except that you  provide  the\n"
 , stdout);
  fputs(
-"       --netrc-file\n"
-"              This  option  is similar to --netrc, except that you provide the\n"
-"              path (absolute or relative) to the netrc file that  Curl  should\n"
-"              use.   You  can  only  specify one netrc file per invocation. If\n"
-"              several --netrc-file options are provided,  only  the  last  one\n"
+"              path  (absolute  or relative) to the netrc file that Curl should\n"
+"              use.  You can only specify one netrc  file  per  invocation.  If\n"
+"              several  --netrc-file  options  are  provided, only the last one\n"
 "              will be used.  (Added in 7.21.5)\n"
 "\n"
-"              This  option  overrides  any use of --netrc as they are mutually\n"
-, stdout);
- fputs(
+"              This option overrides any use of --netrc as  they  are  mutually\n"
 "              exclusive.  It will also abide by --netrc-optional if specified.\n"
 "\n"
 "       --netrc-optional\n"
-"              Very similar to --netrc, but this option makes the .netrc  usage\n"
+, stdout);
+ fputs(
+"              Very  similar to --netrc, but this option makes the .netrc usage\n"
 "              optional and not mandatory as the --netrc option does.\n"
 "\n"
 "       --negotiate\n"
 "              (HTTP) Enables Negotiate (SPNEGO) authentication.\n"
 "\n"
-"              If  you  want to enable Negotiate (SPNEGO) for proxy authentica-\n"
+"              If you want to enable Negotiate (SPNEGO) for  proxy  authentica-\n"
 "              tion, then use --proxy-negotiate.\n"
 "\n"
+"              This  option  requires a library built with GSS-API or SSPI sup-\n"
 , stdout);
  fputs(
-"              This option requires a library built with GSS-API or  SSPI  sup-\n"
-"              port.  Use  -V,  --version  to  see  if  your curl supports GSS-\n"
+"              port. Use -V, --version  to  see  if  your  curl  supports  GSS-\n"
 "              API/SSPI and SPNEGO.\n"
 "\n"
-"              When using this option, you must also provide a fake -u,  --user\n"
-"              option  to  activate the authentication code properly. Sending a\n"
-"              '-u :' is enough as the user  name  and  password  from  the  -u\n"
+"              When  using this option, you must also provide a fake -u, --user\n"
+"              option to activate the authentication code properly.  Sending  a\n"
+"              '-u  :'  is  enough  as  the  user name and password from the -u\n"
 "              option aren't actually used.\n"
 "\n"
-, stdout);
- fputs(
-"              If  this  option  is  used  several times, only the first one is\n"
+"              If this option is used several times,  only  the  first  one  is\n"
 "              used.\n"
 "\n"
+, stdout);
+ fputs(
 "       --no-keepalive\n"
 "              Disables the use of keepalive messages on the TCP connection, as\n"
 "              by default curl enables them.\n"
 "\n"
-"              Note  that  this  is the negated option name documented. You can\n"
+"              Note that this is the negated option name  documented.  You  can\n"
 "              thus use --keepalive to enforce keepalive.\n"
 "\n"
 "       --no-sessionid\n"
-"              (SSL) Disable curl's use of SSL session-ID caching.  By  default\n"
+"              (SSL)  Disable curl's use of SSL session-ID caching.  By default\n"
+"              all transfers are done using the cache. Note that while  nothing\n"
 , stdout);
  fputs(
-"              all  transfers are done using the cache. Note that while nothing\n"
-"              should ever get hurt by attempting  to  reuse  SSL  session-IDs,\n"
+"              should  ever  get  hurt  by attempting to reuse SSL session-IDs,\n"
 "              there seem to be broken SSL implementations in the wild that may\n"
-"              require you to disable this in order for you to succeed.  (Added\n"
+"              require  you to disable this in order for you to succeed. (Added\n"
 "              in 7.16.0)\n"
 "\n"
-"              Note  that  this  is the negated option name documented. You can\n"
+"              Note that this is the negated option name  documented.  You  can\n"
 "              thus use --sessionid to enforce session-ID caching.\n"
 "\n"
+"       --noproxy <no-proxy-list>\n"
 , stdout);
  fputs(
-"       --noproxy <no-proxy-list>\n"
-"              Comma-separated list of hosts which do not use a proxy,  if  one\n"
-"              is  specified.  The only wildcard is a single * character, which\n"
+"              Comma-separated  list  of hosts which do not use a proxy, if one\n"
+"              is specified.  The only wildcard is a single * character,  which\n"
 "              matches all hosts, and effectively disables the proxy. Each name\n"
-"              in  this  list  is matched as either a domain which contains the\n"
-"              hostname, or the hostname itself. For example,  local.com  would\n"
+"              in this list is matched as either a domain  which  contains  the\n"
+"              hostname,  or  the hostname itself. For example, local.com would\n"
+"              match  local.com,  local.com:80,  and  www.local.com,  but   not\n"
 , stdout);
  fputs(
-"              match   local.com,  local.com:80,  and  www.local.com,  but  not\n"
 "              www.notlocal.com.  (Added in 7.19.4).\n"
 "\n"
+"       --connect-to <host:port:connect-to-host:connect-to-port>\n"
+"              For  a  request  to the given \"host:port\" pair, connect to \"con-\n"
+"              nect-to-host:connect-to-port\"  instead.   This  is  suitable  to\n"
+"              direct  the  request  at  a  specific server, e.g. at a specific\n"
+"              cluster node in a cluster of servers.  This option is only  used\n"
+, stdout);
+ fputs(
+"              to  establish  the  network  connection.  It does NOT affect the\n"
+"              hostname/port that is used for TLS/SSL  (e.g.  SNI,  certificate\n"
+"              verification)  or  for  the  application  protocols.  \"host\" and\n"
+"              \"port\" may be the empty string, meaning \"any host/port\".   \"con-\n"
+"              nect-to-host\"  and  \"connect-to-port\"  may  also  be  the  empty\n"
+"              string, meaning \"use the request's  original  host/port\".   This\n"
+, stdout);
+ fputs(
+"              option can be used many times to add many connect rules.  (Added\n"
+"              in 7.49.0).\n"
+"\n"
 "       --ntlm (HTTP) Enables  NTLM  authentication.  The  NTLM  authentication\n"
 "              method was designed by Microsoft and is used by IIS web servers.\n"
 "              It is a proprietary protocol, reverse-engineered by clever  peo-\n"
 "              ple and implemented in curl based on their efforts. This kind of\n"
 , stdout);
  fputs(
