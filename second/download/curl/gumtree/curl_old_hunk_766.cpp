"       --trace-time\n"
"              Prepends  a  time  stamp to each trace or verbose line that curl\n"
, stdout);
 fputs(
"              displays.  (Added in 7.14.0)\n"
"\n"
"       -u, --user <user:password>\n"
"              Specify the user name and password to use for server authentica-\n"
"              tion. Overrides -n, --netrc and --netrc-optional.\n"
"\n"
"              If  you  just give the user name (without entering a colon) curl\n"
"              will prompt for a password.\n"
"\n"
"              If you use an SSPI-enabled curl binary and do  NTLM  authentica-\n"
, stdout);
 fputs(
"              tion,  you  can force curl to pick up the user name and password\n"
"              from your environment by simply specifying a single  colon  with\n"
"              this option: \"-u :\".\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -U, --proxy-user <user:password>\n"
"              Specify  the user name and password to use for proxy authentica-\n"
"              tion.\n"
"\n"
"              If you use an SSPI-enabled curl binary and do  NTLM  authentica-\n"
, stdout);
 fputs(
"              tion,  you  can force curl to pick up the user name and password\n"
"              from your environment by simply specifying a single  colon  with\n"
"              this option: \"-U :\".\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --url <URL>\n"
"              Specify  a  URL  to  fetch. This option is mostly handy when you\n"
"              want to specify URL(s) in a config file.\n"
"\n"
, stdout);
 fputs(
"              This option may be used any number of times.  To  control  where\n"
"              this  URL  is written, use the -o, --output or the -O, --remote-\n"
"              name options.\n"
"       -v, --verbose\n"
"              Makes the fetching more  verbose/talkative.  Mostly  useful  for\n"
"              debugging.  A line starting with '>' means \"header data\" sent by\n"
"              curl, '<' means \"header data\" received by curl that is hidden in\n"
, stdout);
 fputs(
"              normal cases, and a line starting with '*' means additional info\n"
"              provided by curl.\n"
"\n"
"              Note that if you only want  HTTP  headers  in  the  output,  -i,\n"
"              --include might be the option you're looking for.\n"
"\n"
"              If  you think this option still doesn't give you enough details,\n"
"              consider using --trace or --trace-ascii instead.\n"
"\n"
"              This option overrides previous uses of --trace-ascii or --trace.\n"
"\n"
, stdout);
 fputs(
"              Use -s, --silent to make curl quiet.\n"
"\n"
"       -w, --write-out <format>\n"
"              Defines what to display on stdout after a completed and success-\n"
"              ful  operation.  The  format  is a string that may contain plain\n"
"              text mixed with any number of variables. The string can be spec-\n"
"              ified  as \"string\", to get read from a particular file you spec-\n"
"              ify it \"@filename\" and to tell curl  to  read  the  format  from\n"
, stdout);
 fputs(
"              stdin you write \"@-\".\n"
"\n"
"              The  variables  present in the output format will be substituted\n"
"              by the value or text that curl thinks fit, as  described  below.\n"
"              All  variables are specified as %{variable_name} and to output a\n"
"              normal % you just write them as %%. You can output a newline  by\n"
"              using \\n, a carriage return with \\r and a tab space with \\t.\n"
"\n"
, stdout);
 fputs(
"              NOTE: The %-symbol is a special symbol in the win32-environment,\n"
"              where all occurrences of %  must  be  doubled  when  using  this\n"
"              option.\n"
"\n"
"              The variables available are:\n"
"\n"
"              content_type   The  Content-Type  of  the requested document, if\n"
"                             there was any.\n"
"\n"
"              filename_effective\n"
"                             The ultimate filename that curl  writes  out  to.\n"
, stdout);
 fputs(
"                             This  is only meaningful if curl is told to write\n"
"                             to a file  with  the  --remote-name  or  --output\n"
"                             option.  It's most useful in combination with the\n"
"                             --remote-header-name option. (Added in 7.25.1)\n"
"\n"
"              ftp_entry_path The initial path curl ended up in when logging on\n"
"                             to the remote FTP server. (Added in 7.15.4)\n"
"\n"
, stdout);
 fputs(
"              http_code      The numerical response code that was found in the\n"
"                             last retrieved HTTP(S)  or  FTP(s)  transfer.  In\n"
"                             7.18.2  the alias response_code was added to show\n"
"                             the same info.\n"
"\n"
"              http_connect   The numerical code that was  found  in  the  last\n"
"                             response   (from  a  proxy)  to  a  curl  CONNECT\n"
"                             request. (Added in 7.12.4)\n"
"\n"
, stdout);
 fputs(
"              local_ip       The IP address of  the  local  end  of  the  most\n"
"                             recently  done connection - can be either IPv4 or\n"
"                             IPv6 (Added in 7.29.0)\n"
"\n"
"              local_port     The local port number of the most  recently  done\n"
"                             connection (Added in 7.29.0)\n"
"\n"
"              num_connects   Number  of new connects made in the recent trans-\n"
"                             fer. (Added in 7.12.3)\n"
"\n"
, stdout);
 fputs(
"              num_redirects  Number of redirects that  were  followed  in  the\n"
"                             request. (Added in 7.12.3)\n"
"\n"
"              redirect_url   When  an HTTP request was made without -L to fol-\n"
"                             low redirects, this variable will show the actual\n"
"                             URL  a  redirect  would  take  you  to. (Added in\n"
"                             7.18.2)\n"
"\n"
"              remote_ip      The remote IP address of the most  recently  done\n"
, stdout);
 fputs(
"                             connection - can be either IPv4 or IPv6 (Added in\n"
"                             7.29.0)\n"
"\n"
"              remote_port    The remote port number of the most recently  done\n"
"                             connection (Added in 7.29.0)\n"
"\n"
"              size_download  The total amount of bytes that were downloaded.\n"
"\n"
"              size_header    The total amount of bytes of the downloaded head-\n"
"                             ers.\n"
"\n"
, stdout);
 fputs(
"              size_request   The total amount of bytes that were sent  in  the\n"
"                             HTTP request.\n"
"\n"
"              size_upload    The total amount of bytes that were uploaded.\n"
"\n"
"              speed_download The average download speed that curl measured for\n"
"                             the complete download. Bytes per second.\n"
"\n"
"              speed_upload   The average upload speed that curl  measured  for\n"
"                             the complete upload. Bytes per second.\n"
"\n"
, stdout);
 fputs(
"              ssl_verify_result\n"
"                             The  result of the SSL peer certificate verifica-\n"
"                             tion that was requested. 0 means the verification\n"
"                             was successful. (Added in 7.19.0)\n"
"\n"
"              time_appconnect\n"
"                             The  time,  in  seconds,  it  took from the start\n"
"                             until the SSL/SSH/etc  connect/handshake  to  the\n"
, stdout);
 fputs(
"                             remote host was completed. (Added in 7.19.0)\n"
"\n"
"              time_connect   The  time,  in  seconds,  it  took from the start\n"
"                             until the TCP connect  to  the  remote  host  (or\n"
"                             proxy) was completed.\n"
"\n"
"              time_namelookup\n"
"                             The  time,  in  seconds,  it  took from the start\n"
"                             until the name resolving was completed.\n"
"\n"
"              time_pretransfer\n"
, stdout);
 fputs(
"                             The time, in seconds,  it  took  from  the  start\n"
"                             until  the file transfer was just about to begin.\n"
"                             This includes all pre-transfer commands and nego-\n"
"                             tiations that are specific to the particular pro-\n"
"                             tocol(s) involved.\n"
"\n"
"              time_redirect  The time, in seconds, it took for all redirection\n"
, stdout);
 fputs(
"                             steps  include  name lookup, connect, pretransfer\n"
"                             and transfer before  the  final  transaction  was\n"
"                             started.  time_redirect shows the complete execu-\n"
"                             tion time for multiple  redirections.  (Added  in\n"
"                             7.12.3)\n"
"\n"
"              time_starttransfer\n"
"                             The  time,  in  seconds,  it  took from the start\n"
, stdout);
 fputs(
"                             until the first byte was just about to be  trans-\n"
"                             ferred.  This  includes time_pretransfer and also\n"
"                             the time  the  server  needed  to  calculate  the\n"
"                             result.\n"
"\n"
"              time_total     The  total time, in seconds, that the full opera-\n"
"                             tion lasted. The time will be displayed with mil-\n"
"                             lisecond resolution.\n"
"\n"
, stdout);
 fputs(
"              url_effective  The URL that was fetched last. This is most mean-\n"
"                             ingful if you've told curl  to  follow  location:\n"
"                             headers.\n"
"\n"
"       If this option is used several times, the last one will be used.\n"
"\n"
"       -x, --proxy <[protocol://][user:password@]proxyhost[:port]>\n"
"              Use  the  specified HTTP proxy. If the port number is not speci-\n"
"              fied, it is assumed at port 1080.\n"
"\n"
, stdout);
 fputs(
"              This option overrides existing environment  variables  that  set\n"
"              the  proxy  to use. If there's an environment variable setting a\n"
"              proxy, you can set proxy to \"\" to override it.\n"
