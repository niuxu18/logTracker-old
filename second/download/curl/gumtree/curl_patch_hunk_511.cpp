 "              or LDAP.\n"
 "\n"
 "       --socks5-gssapi-service <servicename>\n"
 "              The default service name for a socks server is rcmd/server-fqdn.\n"
 "              This option allows you to change it.\n"
 "\n"
-"              Examples:\n"
-"               --socks5  proxy-name  --socks5-gssapi-service sockd   would use\n"
-"              sockd/proxy-name\n"
-"               --socks5  proxy-name  --socks5-gssapi-service   sockd/real-name\n"
+"              Examples:   --socks5  proxy-name  --socks5-gssapi-service  sockd\n"
+"              would use sockd/proxy-name --socks5 proxy-name  --socks5-gssapi-\n"
 , stdout);
  fputs(
-"              would  use  sockd/real-name  for cases where the proxy-name does\n"
-"              not match the princpal name.\n"
-"               (Added in 7.19.4).\n"
+"              service  sockd/real-name  would  use  sockd/real-name  for cases\n"
+"              where the proxy-name does not match the principal name.   (Added\n"
+"              in 7.19.4).\n"
 "\n"
 "       --socks5-gssapi-nec\n"
-"              As part of the gssapi negotiation a protection mode  is  negoti-\n"
-"              ated.  The  rfc1961  says  in  section 4.3/4.4 it should be pro-\n"
-"              tected, but the NEC  reference  implementation  does  not.   The\n"
-"              option  --socks5-gssapi-nec  allows  the unprotected exchange of\n"
+"              As  part  of the gssapi negotiation a protection mode is negoti-\n"
+"              ated. RFC 1961 says in section 4.3/4.4 it should  be  protected,\n"
+"              but  the  NEC  reference  implementation  does  not.  The option\n"
 , stdout);
  fputs(
-"              the protection mode negotiation. (Added in 7.19.4).\n"
+"              --socks5-gssapi-nec allows the unprotected exchange of the  pro-\n"
+"              tection mode negotiation. (Added in 7.19.4).\n"
 "\n"
 "       --stderr <file>\n"
-"              Redirect all writes to stderr to the specified file instead.  If\n"
-"              the  file  name is a plain '-', it is instead written to stdout.\n"
-"              This option has no point when you're using a shell  with  decent\n"
+"              Redirect  all writes to stderr to the specified file instead. If\n"
+"              the file name is a plain '-', it is instead written  to  stdout.\n"
+"              This  option  has no point when you're using a shell with decent\n"
 "              redirecting capabilities.\n"
 "\n"
+, stdout);
+ fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --tcp-nodelay\n"
-, stdout);
- fputs(
-"              Turn  on the TCP_NODELAY option. See the curl_easy_setopt(3) man\n"
+"              Turn on the TCP_NODELAY option. See the curl_easy_setopt(3)  man\n"
 "              page for details about this option. (Added in 7.11.2)\n"
 "\n"
 "       -t/--telnet-option <OPT=val>\n"
 "              Pass options to the telnet protocol. Supported options are:\n"
 "\n"
 "              TTYPE=<term> Sets the terminal type.\n"
 "\n"
 "              XDISPLOC=<X display> Sets the X display location.\n"
 "\n"
+, stdout);
+ fputs(
 "              NEW_ENV=<var,val> Sets an environment variable.\n"
 "\n"
 "       --tftp-blksize <value>\n"
-, stdout);
- fputs(
 "              (TFTP) Set TFTP BLKSIZE option (must be >512). This is the block\n"
-"              size  that curl will try to use when tranferring data to or from\n"
+"              size that curl will try to use when transferring data to or from\n"
 "              a TFTP server. By default 512 bytes will be used.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "              (Added in 7.20.0)\n"
 "\n"
+"       --tlsauthtype <authtype>\n"
+, stdout);
+ fputs(
+"              Set  TLS  authentication  type.  Currently,  the  only supported\n"
+"              option is \"SRP\",  for  TLS-SRP  (RFC  5054).  If  --tlsuser  and\n"
+"              --tlspassword  are specified but --tlsauthtype is not, then this\n"
+"              option defaults to \"SRP\".  (Added in 7.21.4)\n"
+"\n"
+"       --tlsuser <user>\n"
+"              Set username for use with the TLS authentication  method  speci-\n"
+"              fied  with  --tlsauthtype.  Requires  that --tlspassword also be\n"
+, stdout);
+ fputs(
+"              set.  (Added in 7.21.4)\n"
+"\n"
+"       --tlspassword <password>\n"
+"              Set password for use with the TLS authentication  method  speci-\n"
+"              fied  with  --tlsauthtype.  Requires that --tlsuser also be set.\n"
+"              (Added in 7.21.4)\n"
+"\n"
 "       -T/--upload-file <file>\n"
 "              This transfers the specified local file to the  remote  URL.  If\n"
+"              there is no file part in the specified URL, Curl will append the\n"
 , stdout);
  fputs(
-"              there is no file part in the specified URL, Curl will append the\n"
 "              local file name. NOTE that you must use a trailing / on the last\n"
 "              directory  to really prove to Curl that there is no file name or\n"
 "              curl will think that your last directory name is the remote file\n"
 "              name to use. That will most likely cause the upload operation to\n"
 "              fail. If this is used on a HTTP(S) server, the PUT command  will\n"
 "              be used.\n"
 "\n"
+"              Use  the file name \"-\" (a single dash) to use stdin instead of a\n"
 , stdout);
  fputs(
-"              Use  the file name \"-\" (a single dash) to use stdin instead of a\n"
 "              given file.  Alternately, the file name \".\"  (a  single  period)\n"
 "              may  be  specified  instead  of \"-\" to use stdin in non-blocking\n"
 "              mode to  allow  reading  server  output  while  stdin  is  being\n"
 "              uploaded.\n"
 "\n"
 "              You can specify one -T for each URL on the command line. Each -T\n"
+"              + URL pair specifies what to upload and to where. curl also sup-\n"
 , stdout);
  fputs(
-"              + URL pair specifies what to upload and to where. curl also sup-\n"
 "              ports \"globbing\" of the -T argument, meaning that you can upload\n"
 "              multiple files to a single URL by using the  same  URL  globbing\n"
 "              style supported in the URL, like this:\n"
 "\n"
 "              curl -T \"{file1,file2}\" http://www.uploadtothissite.com\n"
 "\n"
 "              or even\n"
 "\n"
 "              curl -T \"img[1-1000].png\" ftp://ftp.picturemania.com/upload/\n"
 "\n"
 "       --trace <file>\n"
+"              Enables  a  full  trace  dump of all incoming and outgoing data,\n"
 , stdout);
  fputs(
-"              Enables  a  full  trace  dump of all incoming and outgoing data,\n"
 "              including descriptive information, to the given output file. Use\n"
 "              \"-\" as filename to have the output sent to stdout.\n"
 "\n"
 "              This  option overrides previous uses of -v/--verbose or --trace-\n"
 "              ascii.\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --trace-ascii <file>\n"
+"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
 , stdout);
  fputs(
-"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
 "              including descriptive information, to the given output file. Use\n"
 "              \"-\" as filename to have the output sent to stdout.\n"
 "\n"
 "              This is very similar to --trace, but leaves out the hex part and\n"
 "              only  shows  the ASCII part of the dump. It makes smaller output\n"
 "              that might be easier to read for untrained humans.\n"
 "\n"
-, stdout);
- fputs(
 "              This option overrides previous uses of -v/--verbose or --trace.\n"
 "\n"
+, stdout);
+ fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --trace-time\n"
 "              Prepends a time stamp to each trace or verbose  line  that  curl\n"
 "              displays.  (Added in 7.14.0)\n"
 "\n"
 "       -u/--user <user:password>\n"
 "              Specify the user name and password to use for server authentica-\n"
 "              tion. Overrides -n/--netrc and --netrc-optional.\n"
 "\n"
+"              If you just give the user name (without entering a  colon)  curl\n"
 , stdout);
  fputs(
-"              If you just give the user name (without entering a  colon)  curl\n"
 "              will prompt for a password.\n"
 "\n"
 "              If  you  use an SSPI-enabled curl binary and do NTLM authentica-\n"
 "              tion, you can force curl to pick up the user name  and  password\n"
 "              from  your  environment by simply specifying a single colon with\n"
 "              this option: \"-u :\".\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+"       -U/--proxy-user <user:password>\n"
 , stdout);
  fputs(
-"       -U/--proxy-user <user:password>\n"
 "              Specify the user name and password to use for proxy  authentica-\n"
 "              tion.\n"
 "\n"
 "              If  you  use an SSPI-enabled curl binary and do NTLM authentica-\n"
 "              tion, you can force curl to pick up the user name  and  password\n"
 "              from  your  environment by simply specifying a single colon with\n"
 "              this option: \"-U :\".\n"
 "\n"
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+"       --url <URL>\n"
 , stdout);
  fputs(
-"       --url <URL>\n"
 "              Specify a URL to fetch. This option is  mostly  handy  when  you\n"
 "              want to specify URL(s) in a config file.\n"
 "\n"
 "              This  option  may  be used any number of times. To control where\n"
 "              this URL is written, use the -o/--output or the -O/--remote-name\n"
 "              options.\n"
 "\n"
 "       -v/--verbose\n"
 "              Makes  the  fetching  more  verbose/talkative. Mostly useful for\n"
+"              debugging. A line starting with '>' means \"header data\" sent  by\n"
 , stdout);
  fputs(
-"              debugging. A line starting with '>' means \"header data\" sent  by\n"
 "              curl, '<' means \"header data\" received by curl that is hidden in\n"
 "              normal cases, and a line starting with '*' means additional info\n"
 "              provided by curl.\n"
 "\n"
 "              Note  that  if  you  only  want  HTTP  headers  in  the  output,\n"
 "              -i/--include might be the option you're looking for.\n"
 "\n"
 "              If you think this option still doesn't give you enough  details,\n"
-, stdout);
- fputs(
 "              consider using --trace or --trace-ascii instead.\n"
 "\n"
+, stdout);
+ fputs(
 "              This option overrides previous uses of --trace-ascii or --trace.\n"
 "\n"
-"              Use -S/--silent to make curl quiet.\n"
+"              Use -s/--silent to make curl quiet.\n"
 "\n"
 "       -V/--version\n"
 "              Displays information about curl and the libcurl version it uses.\n"
 "              The  first  line  includes the full version of curl, libcurl and\n"
 "              other 3rd party libraries linked with the executable.\n"
 "\n"
+"              The second line (starts with \"Protocols:\") shows  all  protocols\n"
 , stdout);
  fputs(
-"              The second line (starts with \"Protocols:\") shows  all  protocols\n"
 "              that libcurl reports to support.\n"
 "\n"
 "              The third line (starts with \"Features:\") shows specific features\n"
 "              libcurl reports to offer. Available features include:\n"
 "\n"
 "              IPv6   You can use IPv6 with this.\n"
 "\n"
 "              krb4   Krb4 for FTP is supported.\n"
 "\n"
 "              SSL    HTTPS and FTPS are supported.\n"
 "\n"
-, stdout);
- fputs(
 "              libz   Automatic decompression of compressed files over HTTP  is\n"
 "                     supported.\n"
 "\n"
+, stdout);
+ fputs(
 "              NTLM   NTLM authentication is supported.\n"
 "\n"
 "              GSS-Negotiate\n"
 "                     Negotiate authentication and krb5 for FTP is supported.\n"
 "\n"
 "              Debug  This  curl  uses a libcurl built with Debug. This enables\n"
 "                     more error-tracking and memory debugging etc.  For  curl-\n"
 "                     developers only!\n"
 "\n"
 "              AsynchDNS\n"
-, stdout);
- fputs(
 "                     This curl uses asynchronous name resolves.\n"
 "\n"
+, stdout);
+ fputs(
 "              SPNEGO SPNEGO Negotiate authentication is supported.\n"
 "\n"
 "              Largefile\n"
 "                     This curl supports transfers of large files, files larger\n"
 "                     than 2GB.\n"
 "\n"
 "              IDN    This curl supports IDN - international domain names.\n"
 "\n"
 "              SSPI   SSPI is supported. If you use NTLM and set a  blank  user\n"
-, stdout);
- fputs(
 "                     name,  curl  will authenticate with your current user and\n"
 "                     password.\n"
 "\n"
+"              TLS-SRP\n"
+, stdout);
+ fputs(
+"                     SRP (Secure Remote Password) authentication is  supported\n"
+"                     for TLS.\n"
+"\n"
 "       -w/--write-out <format>\n"
 "              Defines what to display on stdout after a completed and success-\n"
-"              ful  operation.  The  format  is a string that may contain plain\n"
+"              ful operation. The format is a string  that  may  contain  plain\n"
 "              text mixed with any number of variables. The string can be spec-\n"
-"              ified  as \"string\", to get read from a particular file you spec-\n"
+"              ified as \"string\", to get read from a particular file you  spec-\n"
 , stdout);
  fputs(
-"              ify it \"@filename\" and to tell curl  to  read  the  format  from\n"
+"              ify  it  \"@filename\"  and  to  tell curl to read the format from\n"
 "              stdin you write \"@-\".\n"
 "\n"
-"              The  variables  present in the output format will be substituted\n"
-"              by the value or text that curl thinks fit, as  described  below.\n"
-"              All  variables are specified as %{variable_name} and to output a\n"
-"              normal % you just write them as %%. You can output a newline  by\n"
+"              The variables present in the output format will  be  substituted\n"
+"              by  the  value or text that curl thinks fit, as described below.\n"
+"              All variables are specified as %{variable_name} and to output  a\n"
+"              normal  % you just write them as %%. You can output a newline by\n"
 , stdout);
  fputs(
 "              using \\n, a carriage return with \\r and a tab space with \\t.\n"
 "\n"
 "              NOTE: The %-symbol is a special symbol in the win32-environment,\n"
-"              where all occurrences of %  must  be  doubled  when  using  this\n"
+"              where  all  occurrences  of  %  must  be doubled when using this\n"
 "              option.\n"
 "\n"
 "              The variables available at this point are:\n"
 "\n"
 "              url_effective  The URL that was fetched last. This is most mean-\n"
-"                             ingful if you've told curl  to  follow  location:\n"
+"                             ingful  if  you've  told curl to follow location:\n"
 , stdout);
  fputs(
 "                             headers.\n"
 "\n"
 "              http_code      The numerical response code that was found in the\n"
-"                             last retrieved HTTP(S)  or  FTP(s)  transfer.  In\n"
-"                             7.18.2  the alias response_code was added to show\n"
+"                             last  retrieved  HTTP(S)  or  FTP(s) transfer. In\n"
+"                             7.18.2 the alias response_code was added to  show\n"
 "                             the same info.\n"
 "\n"
-"              http_connect   The numerical code that was  found  in  the  last\n"
-"                             response   (from  a  proxy)  to  a  curl  CONNECT\n"
+"              http_connect   The  numerical  code  that  was found in the last\n"
+"                             response  (from  a  proxy)  to  a  curl   CONNECT\n"
 , stdout);
  fputs(
 "                             request. (Added in 7.12.4)\n"
 "\n"
-"              time_total     The total time, in seconds, that the full  opera-\n"
+"              time_total     The  total time, in seconds, that the full opera-\n"
 "                             tion lasted. The time will be displayed with mil-\n"
 "                             lisecond resolution.\n"
 "\n"
 "              time_namelookup\n"
-"                             The time, in seconds,  it  took  from  the  start\n"
+"                             The  time,  in  seconds,  it  took from the start\n"
 "                             until the name resolving was completed.\n"
 "\n"
 , stdout);
  fputs(
-"              time_connect   The  time,  in  seconds,  it  took from the start\n"
-"                             until the TCP connect  to  the  remote  host  (or\n"
+"              time_connect   The time, in seconds,  it  took  from  the  start\n"
+"                             until  the  TCP  connect  to  the remote host (or\n"
 "                             proxy) was completed.\n"
 "\n"
 "              time_appconnect\n"
-"                             The  time,  in  seconds,  it  took from the start\n"
-"                             until the SSL/SSH/etc  connect/handshake  to  the\n"
+"                             The time, in seconds,  it  took  from  the  start\n"
+"                             until  the  SSL/SSH/etc  connect/handshake to the\n"
 "                             remote host was completed. (Added in 7.19.0)\n"
 "\n"
 , stdout);
  fputs(
 "              time_pretransfer\n"
-"                             The  time,  in  seconds,  it  took from the start\n"
-"                             until the file transfer was just about to  begin.\n"
+"                             The time, in seconds,  it  took  from  the  start\n"
+"                             until  the file transfer was just about to begin.\n"
 "                             This includes all pre-transfer commands and nego-\n"
 "                             tiations that are specific to the particular pro-\n"
 "                             tocol(s) involved.\n"
 "\n"
 "              time_redirect  The time, in seconds, it took for all redirection\n"
 , stdout);
  fputs(
-"                             steps include name lookup,  connect,  pretransfer\n"
-"                             and  transfer  before  the  final transaction was\n"
-"                             started. time_redirect shows the complete  execu-\n"
-"                             tion  time  for  multiple redirections. (Added in\n"
+"                             steps  include  name lookup, connect, pretransfer\n"
+"                             and transfer before  the  final  transaction  was\n"
+"                             started.  time_redirect shows the complete execu-\n"
+"                             tion time for multiple  redirections.  (Added  in\n"
 "                             7.12.3)\n"
 "\n"
 "              time_starttransfer\n"
-"                             The time, in seconds,  it  took  from  the  start\n"
+"                             The  time,  in  seconds,  it  took from the start\n"
 , stdout);
  fputs(
-"                             until  the first byte was just about to be trans-\n"
-"                             ferred. This includes time_pretransfer  and  also\n"
-"                             the  time  the  server  needed  to  calculate the\n"
+"                             until the first byte was just about to be  trans-\n"
+"                             ferred.  This  includes time_pretransfer and also\n"
+"                             the time  the  server  needed  to  calculate  the\n"
 "                             result.\n"
 "\n"
 "              size_download  The total amount of bytes that were downloaded.\n"
 "\n"
 "              size_upload    The total amount of bytes that were uploaded.\n"
 "\n"
 , stdout);
  fputs(
 "              size_header    The total amount of bytes of the downloaded head-\n"
 "                             ers.\n"
 "\n"
-"              size_request   The  total  amount of bytes that were sent in the\n"
+"              size_request   The total amount of bytes that were sent  in  the\n"
 "                             HTTP request.\n"
 "\n"
 "              speed_download The average download speed that curl measured for\n"
 "                             the complete download. Bytes per second.\n"
 "\n"
-"              speed_upload   The  average  upload speed that curl measured for\n"
+"              speed_upload   The average upload speed that curl  measured  for\n"
 , stdout);
  fputs(
 "                             the complete upload. Bytes per second.\n"
 "\n"
-"              content_type   The Content-Type of the  requested  document,  if\n"
+"              content_type   The  Content-Type  of  the requested document, if\n"
 "                             there was any.\n"
 "\n"
-"              num_connects   Number  of new connects made in the recent trans-\n"
+"              num_connects   Number of new connects made in the recent  trans-\n"
 "                             fer. (Added in 7.12.3)\n"
 "\n"
-"              num_redirects  Number of redirects that  were  followed  in  the\n"
+"              num_redirects  Number  of  redirects  that  were followed in the\n"
 "                             request. (Added in 7.12.3)\n"
 "\n"
 , stdout);
  fputs(
 "              redirect_url   When a HTTP request was made without -L to follow\n"
 "                             redirects, this variable will show the actual URL\n"
