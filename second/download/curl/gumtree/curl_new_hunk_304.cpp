"       -s/--silent\n"
, stdout);
 fputs(
"              Silent mode. Don't show progress meter or error messages.  Makes\n"
"              Curl mute.\n"
"\n"
"       -S/--show-error\n"
"              When used with -s it makes curl show error message if it  fails.\n"
"\n"
"       --socks4 <host[:port]>\n"
"              Use the specified SOCKS4 proxy. If the port number is not speci-\n"
"              fied, it is assumed at port 1080. (Added in 7.15.2)\n"
"\n"
"              This option overrides any previous use of  -x/--proxy,  as  they\n"
, stdout);
 fputs(
"              are mutually exclusive.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --socks4a <host[:port]>\n"
"              Use the specified SOCKS4a proxy. If the port number is not spec-\n"
"              ified, it is assumed at port 1080. (Added in 7.18.0)\n"
"\n"
"              This  option  overrides  any previous use of -x/--proxy, as they\n"
"              are mutually exclusive.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
, stdout);
 fputs(
"       --socks5-hostname <host[:port]>\n"
"              Use  the  specified  SOCKS5 proxy (and let the proxy resolve the\n"
"              host name). If the port number is not specified, it  is  assumed\n"
"              at port 1080. (Added in 7.18.0)\n"
"\n"
"              This  option  overrides  any previous use of -x/--proxy, as they\n"
"              are mutually exclusive.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
, stdout);
 fputs(
"              (This  option  was  previously  wrongly  documented  and used as\n"
"              --socks without the number appended.)\n"
"\n"
"       --socks5 <host[:port]>\n"
"              Use the specified SOCKS5 proxy  -  but  resolve  the  host  name\n"
"              locally.  If  the port number is not specified, it is assumed at\n"
"              port 1080.\n"
"\n"
"              This option overrides any previous use of  -x/--proxy,  as  they\n"
"              are mutually exclusive.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"              (This option was  previously  wrongly  documented  and  used  as\n"
"              --socks without the number appended.)\n"
"\n"
"       --stderr <file>\n"
"              Redirect  all writes to stderr to the specified file instead. If\n"
"              the file name is a plain '-', it is instead written  to  stdout.\n"
"              This  option  has no point when you're using a shell with decent\n"
, stdout);
 fputs(
"              redirecting capabilities.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --tcp-nodelay\n"
"              Turn  on the TCP_NODELAY option. See the curl_easy_setopt(3) man\n"
"              page for details about this option. (Added in 7.11.2)\n"
"\n"
"       -t/--telnet-option <OPT=val>\n"
"              Pass options to the telnet protocol. Supported options are:\n"
"\n"
"              TTYPE=<term> Sets the terminal type.\n"
"\n"
, stdout);
 fputs(
"              XDISPLOC=<X display> Sets the X display location.\n"
"\n"
"              NEW_ENV=<var,val> Sets an environment variable.\n"
"\n"
"       -T/--upload-file <file>\n"
"              This transfers the specified local file to the  remote  URL.  If\n"
"              there is no file part in the specified URL, Curl will append the\n"
"              local file name. NOTE that you must use a trailing / on the last\n"
"              directory  to really prove to Curl that there is no file name or\n"
, stdout);
 fputs(
"              curl will think that your last directory name is the remote file\n"
"              name to use. That will most likely cause the upload operation to\n"
"              fail. If this is used on a http(s) server, the PUT command  will\n"
"              be used.\n"
"\n"
"              Use  the file name \"-\" (a single dash) to use stdin instead of a\n"
"              given file.\n"
"\n"
"              You can specify one -T for each URL on the command line. Each -T\n"
, stdout);
 fputs(
"              + URL pair specifies what to upload and to where. curl also sup-\n"
"              ports \"globbing\" of the -T argument, meaning that you can upload\n"
"              multiple  files  to  a single URL by using the same URL globbing\n"
"              style supported in the URL, like this:\n"
"\n"
"              curl -T \"{file1,file2}\" http://www.uploadtothissite.com\n"
"\n"
"              or even\n"
"\n"
"              curl -T \"img[1-1000].png\" ftp://ftp.picturemania.com/upload/\n"
"\n"
"       --trace <file>\n"
, stdout);
 fputs(
"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
"              including descriptive information, to the given output file. Use\n"
"              \"-\" as filename to have the output sent to stdout.\n"
"\n"
"              This option overrides previous uses of -v/--verbose or  --trace-\n"
"              ascii.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --trace-ascii <file>\n"
, stdout);
 fputs(
"              Enables a full trace dump of all  incoming  and  outgoing  data,\n"
"              including descriptive information, to the given output file. Use\n"
"              \"-\" as filename to have the output sent to stdout.\n"
"\n"
"              This is very similar to --trace, but leaves out the hex part and\n"
"              only  shows  the ASCII part of the dump. It makes smaller output\n"
"              that might be easier to read for untrained humans.\n"
"\n"
, stdout);
 fputs(
"              This option overrides previous uses of -v/--verbose or  --trace.\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --trace-time\n"
"              Prepends a time stamp to each trace or verbose  line  that  curl\n"
"              displays.  (Added in 7.14.0)\n"
"\n"
"       -u/--user <user:password>\n"
"              Specify  user  and  password  to  use for server authentication.\n"
"              Overrides -n/--netrc and --netrc-optional.\n"
"\n"
, stdout);
 fputs(
"              If you just give the user name (without entering a  colon)  curl\n"
"              will prompt for a password.\n"
"\n"
"              If  you  use an SSPI-enabled curl binary and do NTLM authentica-\n"
"              tion, you can force curl to pick up the user name  and  password\n"
"              from  your  environment by simply specifying a single colon with\n"
"              this option: \"-u :\".\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
, stdout);
 fputs(
"       -U/--proxy-user <user:password>\n"
"              Specify user and password to use for proxy authentication.\n"
"\n"
"              If  you  use an SSPI-enabled curl binary and do NTLM authentica-\n"
"              tion, you can force curl to pick up the user name  and  password\n"
"              from  your  environment by simply specifying a single colon with\n"
"              this option: \"-U :\".\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --url <URL>\n"
, stdout);
 fputs(
"              Specify  a  URL  to  fetch. This option is mostly handy when you\n"
"              want to specify URL(s) in a config file.\n"
"\n"
"              This option may be used any number of times.  To  control  where\n"
"              this URL is written, use the -o/--output or the -O/--remote-name\n"
"              options.\n"
"\n"
"       -v/--verbose\n"
"              Makes the fetching more  verbose/talkative.  Mostly  usable  for\n"
"              debugging.  Lines  starting with '>' means \"header data\" sent by\n"
, stdout);
 fputs(
"              curl, '<' means \"header data\" received by curl that is hidden in\n"
"              normal  cases  and lines starting with '*' means additional info\n"
"              provided by curl.\n"
"\n"
"              Note  that  if  you  only  want  HTTP  headers  in  the  output,\n"
"              -i/--include might be option you're looking for.\n"
"\n"
"              If  you think this option still doesn't give you enough details,\n"
"              consider using --trace or --trace-ascii instead.\n"
"\n"
, stdout);
 fputs(
"              This option overrides previous uses of --trace-ascii or --trace.\n"
"\n"
"       -V/--version\n"
"              Displays information about curl and the libcurl version it uses.\n"
"              The first line includes the full version of  curl,  libcurl  and\n"
"              other 3rd party libraries linked with the executable.\n"
"\n"
"              The  second  line (starts with \"Protocols:\") shows all protocols\n"
"              that libcurl reports to support.\n"
"\n"
, stdout);
 fputs(
"              The third line (starts with \"Features:\") shows specific features\n"
"              libcurl reports to offer. Available features include:\n"
