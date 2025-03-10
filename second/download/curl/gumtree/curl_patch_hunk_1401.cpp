 "          0-0,-1    specifies the first and last byte only(*)(H)\n"
 "\n"
 "          500-700,600-799\n"
 "                    specifies 300 bytes from offset 500(H)\n"
 "\n"
 "          100-199,500-599\n"
-"                    specifies two separate 100 bytes ranges(*)(H)\n"
 );
  puts(
-"     (*) = NOTE that this will cause the server to reply with a\n"
+"                    specifies two separate 100 bytes ranges(*)(H)\n"
+"\n"
+"     (*) = NOTE that this will cause the server to reply  with  a\n"
 "     multipart response!\n"
 "\n"
-"     You should also be aware that many HTTP/1.1 servers do not\n"
+"     You  should  also be aware that many HTTP/1.1 servers do not\n"
 "     have this feature enabled, so that when you attempt to get a\n"
 "     range, you'll instead get the whole document.\n"
 "\n"
-"     FTP range downloads only support the simple syntax 'start-\n"
-"     stop' (optionally with one of the numbers omitted). It\n"
-"     depends on the non-RFC command SIZE.\n"
-"\n"
+"     FTP  range  downloads only support the simple syntax 'start-\n"
+"     stop' (optionally with  one  of  the  numbers  omitted).  It\n"
 );
  puts(
-"     If this option is used serveral times, the last one will be\n"
+"     depends on the non-RFC command SIZE.\n"
+"\n"
+"     If  this option is used serveral times, the last one will be\n"
 "     used.\n"
 "\n"
 "     -s/--silent\n"
-"          Silent mode. Don't show progress meter or error mes�\n"
+"          Silent mode. Don't show progress meter  or  error  mes�\n"
 "          sages.  Makes Curl mute.\n"
 "\n"
-"          If this option is used twice, the second will again\n"
+"          If  this  option  is  used twice, the second will again\n"
 "          disable mute.\n"
 "\n"
 "     -S/--show-error\n"
-"          When used with -s it makes curl show error message if\n"
+"          When used with -s it makes curl show error  message  if\n"
 "          it fails.\n"
 "\n"
-"          If this option is used twice, the second will again\n"
-"          disable show error.\n"
-"\n"
 );
  puts(
+"          If  this  option  is  used twice, the second will again\n"
+"          disable show error.\n"
+"\n"
 "     -t/--telnet-option <OPT=val>\n"
-"          Pass options to the telnet protocol. Supported options\n"
+"          Pass options to the telnet protocol. Supported  options\n"
 "          are:\n"
 "\n"
 "          TTYPE=<term> Sets the terminal type.\n"
 "\n"
 "          XDISPLOC=<X display> Sets the X display location.\n"
 "\n"
 "          NEW_ENV=<var,val> Sets an environment variable.\n"
 "\n"
 "     -T/--upload-file <file>\n"
-"          Like -t, but this transfers the specified local file.\n"
-"          If there is no file part in the specified URL, Curl\n"
+"          Like  -t,  but this transfers the specified local file.\n"
 );
  puts(
+"          If there is no file part in  the  specified  URL,  Curl\n"
 "          will append the local file name. NOTE that you must use\n"
-"          a trailing / on the last directory to really prove to\n"
+"          a trailing / on the last directory to really  prove  to\n"
 "          Curl that there is no file name or curl will think that\n"
-"          your last directory name is the remote file name to\n"
-"          use. That will most likely cause the upload operation\n"
-"          to fail. If this is used on a http(s) server, the PUT\n"
+"          your last directory name is the  remote  file  name  to\n"
+"          use.  That  will most likely cause the upload operation\n"
+"          to fail. If this is used on a http(s) server,  the  PUT\n"
 "          command will be used.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
 );
  puts(
+"          If  this  option  is  used serveral times, the last one\n"
 "          will be used.\n"
+"\n"
 "     -u/--user <user:password>\n"
-"          Specify user and password to use when fetching. See\n"
-"          README.curl for detailed examples of how to use this.\n"
-"          If no password is specified, curl will ask for it\n"
+"          Specify user and password to  use  when  fetching.  See\n"
+"          README.curl  for  detailed examples of how to use this.\n"
+"          If no password is  specified,  curl  will  ask  for  it\n"
 "          interactively.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+"          If  this  option  is  used serveral times, the last one\n"
 "          will be used.\n"
 "\n"
 "     -U/--proxy-user <user:password>\n"
-"          Specify user and password to use for Proxy authentica�\n"
 );
  puts(
+"          Specify user and password to use for Proxy  authentica�\n"
 "          tion. If no password is specified, curl will ask for it\n"
 "          interactively.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+"          If this option is used serveral  times,  the  last  one\n"
 "          will be used.\n"
 "\n"
 "     --url <URL>\n"
-"          Specify a URL to fetch. This option is mostly handy\n"
+"          Specify  a  URL  to  fetch. This option is mostly handy\n"
 "          when you wanna specify URL(s) in a config file.\n"
 "\n"
 "          This option may be used any number of times. To control\n"
-"          where this URL is written, use the -o or the -O\n"
+);
+ puts(
+"          where  this  URL  is  written,  use  the  -o  or the -O\n"
 "          options.\n"
 "\n"
 "     -v/--verbose\n"
-);
- puts(
-"          Makes the fetching more verbose/talkative. Mostly\n"
-"          usable for debugging. Lines starting with '>' means\n"
+"          Makes  the  fetching  more  verbose/talkative.   Mostly\n"
+"          usable  for  debugging.  Lines  starting with '>' means\n"
 "          data sent by curl, '<' means data received by curl that\n"
-"          is hidden in normal cases and lines starting with '*'\n"
+"          is  hidden  in normal cases and lines starting with '*'\n"
 "          means additional info provided by curl.\n"
 "\n"
-"          If this option is used twice, the second will again\n"
+"          If this option is used twice,  the  second  will  again\n"
+);
+ puts(
 "          disable verbose.\n"
 "\n"
 "     -V/--version\n"
-"          Displays the full version of curl, libcurl and other\n"
-);
- puts(
+"          Displays  the  full  version of curl, libcurl and other\n"
 "          3rd party libraries linked with the executable.\n"
 "\n"
 "     -w/--write-out <format>\n"
-"          Defines what to display after a completed and success�\n"
-"          ful operation. The format is a string that may contain\n"
-"          plain text mixed with any number of variables. The\n"
+"          Defines what to display after a completed and  success�\n"
+"          ful  operation. The format is a string that may contain\n"
+"          plain text mixed with  any  number  of  variables.  The\n"
 "          string can be specified as \"string\", to get read from a\n"
-"          particular file you specify it \"@filename\" and to tell\n"
+);
+ puts(
+"          particular file you specify it \"@filename\" and to  tell\n"
 "          curl to read the format from stdin you write \"@-\".\n"
 "\n"
+"          The variables present in the output format will be sub�\n"
+"          stituted by the value or text that curl thinks fit,  as\n"
+"          described  below.  All  variables  are  specified  like\n"
+"          %{variable_name} and to output  a  normal  %  you  just\n"
+"          write  them  like %%. You can output a newline by using\n"
 );
  puts(
-"          The variables present in the output format will be sub�\n"
-"          stituted by the value or text that curl thinks fit, as\n"
-"          described below. All variables are specified like\n"
-"          %{variable_name} and to output a normal % you just\n"
-"          write them like %%. You can output a newline by using\n"
 "          \\n, a carrige return with \\r and a tab space with \\t.\n"
 "\n"
-"          NOTE: The %-letter is a special letter in the\n"
-);
- puts(
-"          win32-environment, where all occurrences of % must be\n"
+"          NOTE:  The  %-letter  is  a  special  letter   in   the\n"
+"          win32-environment,  where  all occurrences of % must be\n"
 "          doubled when using this option.\n"
 "\n"
 "          Available variables are at this point:\n"
 "\n"
-"          url_effective  The URL that was fetched last. This is\n"
+"          url_effective  The URL that was fetched last.  This  is\n"
 "                         mostly meaningful if you've told curl to\n"
 "                         follow location: headers.\n"
 "\n"
+);
+ puts(
 "          http_code      The numerical code that was found in the\n"
 "                         last retrieved HTTP(S) page.\n"
 "\n"
-);
- puts(
-"          time_total     The total time, in seconds, that the\n"
-"                         full operation lasted. The time will be\n"
+"          time_total     The  total  time,  in  seconds, that the\n"
+"                         full operation lasted. The time will  be\n"
 "                         displayed with millisecond resolution.\n"
 "\n"
 "          time_namelookup\n"
-"                         The time, in seconds, it took from the\n"
-"                         start until the name resolving was com�\n"
-"                         pleted.\n"
-"\n"
-"          time_connect   The time, in seconds, it took from the\n"
+"                         The  time,  in seconds, it took from the\n"
+"                         start until the name resolving was  com�\n"
 );
  puts(
-"                         start until the connect to the remote\n"
+"                         pleted.\n"
+"\n"
+"          time_connect   The  time,  in seconds, it took from the\n"
+"                         start until the connect  to  the  remote\n"
 "                         host (or proxy) was completed.\n"
 "\n"
 "          time_pretransfer\n"
-"                         The time, in seconds, it took from the\n"
-"                         start until the file transfer is just\n"
-"                         about to begin. This includes all pre-\n"
-"                         transfer commands and negotiations that\n"
-"                         are specific to the particular proto�\n"
+"                         The  time,  in seconds, it took from the\n"
+"                         start until the file  transfer  is  just\n"
+"                         about  to  begin. This includes all pre-\n"
 );
  puts(
+"                         transfer commands and negotiations  that\n"
+"                         are  specific  to  the particular proto�\n"
 "                         col(s) involved.\n"
 "\n"
-"          size_download  The total amount of bytes that were\n"
+"          size_download  The total  amount  of  bytes  that  were\n"
 "                         downloaded.\n"
 "\n"
-"          size_upload    The total amount of bytes that were\n"
+"          size_upload    The  total  amount  of  bytes  that were\n"
 "                         uploaded.\n"
 "\n"
-"          size_header    The total amount of bytes of the down�\n"
+"          size_header    The total amount of bytes of  the  down�\n"
 "                         loaded headers.\n"
 "\n"
+);
+ puts(
 "          size_request   The total amount of bytes that were sent\n"
 "                         in the HTTP request.\n"
 "\n"
-);
- puts(
-"          speed_download The average download speed that curl\n"
+"          speed_download The average  download  speed  that  curl\n"
 "                         measured for the complete download.\n"
+"          speed_upload   The  average upload speed that curl mea�\n"
+"                         sured for the complete upload.\n"
 "\n"
-"          speed_upload   The average upload speed that curl\n"
-"                         measured for the complete upload.\n"
-"\n"
-"     If this option is used serveral times, the last one will be\n"
+"     If this option is used serveral times, the last one will  be\n"
 "     used.\n"
 "\n"
 "     -x/--proxy <proxyhost[:port]>\n"
-"          Use specified proxy. If the port number is not speci�\n"
-"          fied, it is assumed at port 1080.\n"
-"\n"
 );
  puts(
-"          If this option is used serveral times, the last one\n"
+"          Use  specified  proxy. If the port number is not speci�\n"
+"          fied, it is assumed at port 1080.\n"
+"\n"
+"          If this option is used serveral  times,  the  last  one\n"
 "          will be used.\n"
 "\n"
 "     -X/--request <command>\n"
-"          (HTTP) Specifies a custom request to use when communi�\n"
-"          cating with the HTTP server.  The specified request\n"
+"          (HTTP)  Specifies a custom request to use when communi�\n"
+"          cating with the HTTP  server.   The  specified  request\n"
 "          will be used instead of the standard GET. Read the HTTP\n"
 "          1.1 specification for details and explanations.\n"
 "\n"
-"          (FTP) Specifies a custom FTP command to use instead of\n"
-"          LIST when doing file lists with ftp.\n"
-"\n"
 );
  puts(
-"          If this option is used serveral times, the last one\n"
+"          (FTP) Specifies a custom FTP command to use instead  of\n"
+"          LIST when doing file lists with ftp.\n"
+"\n"
+"          If  this  option  is  used serveral times, the last one\n"
 "          will be used.\n"
 "\n"
 "     -y/--speed-time <time>\n"
 "          If a download is slower than speed-limit bytes per sec�\n"
-"          ond during a speed-time period, the download gets\n"
+"          ond  during  a  speed-time  period,  the  download gets\n"
 "          aborted. If speed-time is used, the default speed-limit\n"
 "          will be 1 unless set with -y.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+);
+ puts(
+"          If  this  option  is  used serveral times, the last one\n"
 "          will be used.\n"
 "\n"
 "     -Y/--speed-limit <speed>\n"
-);
- puts(
 "          If a download is slower than this given speed, in bytes\n"
-"          per second, for speed-time seconds it gets aborted.\n"
+"          per  second,  for  speed-time  seconds it gets aborted.\n"
 "          speed-time is set with -Y and is 30 if not set.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+"          If this option is used serveral  times,  the  last  one\n"
 "          will be used.\n"
 "\n"
 "     -z/--time-cond <date expression>\n"
-"          (HTTP) Request to get a file that has been modified\n"
-"          later than the given time and date, or one that has\n"
 );
  puts(
+"          (HTTP)  Request  to  get  a file that has been modified\n"
+"          later than the given time and date,  or  one  that  has\n"
 "          been modified before that time. The date expression can\n"
 "          be all sorts of date strings or if it doesn't match any\n"
-"          internal ones, it tries to get the time from a given\n"
-"          file name instead! See the GNU date(1) or curl_get�\n"
+"          internal  ones,  it  tries to get the time from a given\n"
+"          file name instead! See the  GNU  date(1)  or  curl_get�\n"
 "          date(3) man pages for date expression details.\n"
 "\n"
-"          Start the date expression with a dash (-) to make it\n"
-"          request for a document that is older than the given\n"
 );
  puts(
+"          Start  the  date  expression with a dash (-) to make it\n"
+"          request for a document that is  older  than  the  given\n"
 "          date/time, default is a document that is newer than the\n"
 "          specified date/time.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+"          If this option is used serveral  times,  the  last  one\n"
 "          will be used.\n"
 "\n"
 "     -3/--sslv3\n"
 "          (HTTPS) Forces curl to use SSL version 3 when negotiat�\n"
 "          ing with a remote SSL server.\n"
 "\n"
 "     -2/--sslv2\n"
+);
+ puts(
 "          (HTTPS) Forces curl to use SSL version 2 when negotiat�\n"
 "          ing with a remote SSL server.\n"
 "\n"
 "     -#/--progress-bar\n"
-);
- puts(
-"          Make curl display progress information as a progress\n"
+"          Make  curl  display  progress information as a progress\n"
 "          bar instead of the default statistics.\n"
 "\n"
-"          If this option is used twice, the second will again\n"
+"          If this option is used twice,  the  second  will  again\n"
 "          disable the progress bar.\n"
 "\n"
 "     --crlf\n"
-"          (FTP) Convert LF to CRLF in upload. Useful for MVS\n"
+"          (FTP)  Convert  LF  to  CRLF  in upload. Useful for MVS\n"
 "          (OS/390).\n"
 "\n"
-"          If this option is used twice, the second will again\n"
+);
+ puts(
+"          If this option is used twice,  the  second  will  again\n"
 "          disable crlf converting.\n"
 "\n"
 "     --stderr <file>\n"
-"          Redirect all writes to stderr to the specified file\n"
-);
- puts(
+"          Redirect  all  writes  to  stderr to the specified file\n"
 "          instead. If the file name is a plain '-', it is instead\n"
 "          written to stdout. This option has no point when you're\n"
 "          using a shell with decent redirecting capabilities.\n"
 "\n"
-"          If this option is used serveral times, the last one\n"
+"          If this option is used serveral  times,  the  last  one\n"
 "          will be used.\n"
 "\n"
 "FILES\n"
 "     ~/.curlrc\n"
+);
+ puts(
 "          Default config file.\n"
 "\n"
 "ENVIRONMENT\n"
 "     HTTP_PROXY [protocol://]<host>[:port]\n"
 "          Sets proxy server to use for HTTP.\n"
 "\n"
 "     HTTPS_PROXY [protocol://]<host>[:port]\n"
-);
- puts(
 "          Sets proxy server to use for HTTPS.\n"
 "\n"
 "     FTP_PROXY [protocol://]<host>[:port]\n"
 "          Sets proxy server to use for FTP.\n"
 "     GOPHER_PROXY [protocol://]<host>[:port]\n"
 "          Sets proxy server to use for GOPHER.\n"
 "\n"
 "     ALL_PROXY [protocol://]<host>[:port]\n"
-"          Sets proxy server to use if no protocol-specific proxy\n"
+);
+ puts(
+"          Sets  proxy server to use if no protocol-specific proxy\n"
 "          is set.\n"
 "\n"
 "     NO_PROXY <comma-separated list of hosts>\n"
 "          list of host names that shouldn't go through any proxy.\n"
-);
- puts(
 "          If set to a asterisk '*' only, it matches all hosts.\n"
 "\n"
 "     COLUMNS <integer>\n"
-"          The width of the terminal.  This variable only affects\n"
+"          The  width of the terminal.  This variable only affects\n"
 "          curl when the --progress-bar option is used.\n"
 "\n"
 "EXIT CODES\n"
 "     There exists a bunch of different error codes and their cor�\n"
-"     responding error messages that may appear during bad condi�\n"
+);
+ puts(
+"     responding  error messages that may appear during bad condi�\n"
 "     tions. At the time of this writing, the exit codes are:\n"
 "\n"
 "     1    Unsupported protocol. This build of curl has no support\n"
-);
- puts(
 "          for this protocol.\n"
 "\n"
 "     2    Failed to initialize.\n"
 "\n"
 "     3    URL malformat. The syntax was not correct.\n"
 "\n"
-"     4    URL user malformatted. The user-part of the URL syntax\n"
+"     4    URL  user malformatted. The user-part of the URL syntax\n"
 "          was not correct.\n"
 "\n"
-"     5    Couldn't resolve proxy. The given proxy host could not\n"
+"     5    Couldn't resolve proxy. The given proxy host could  not\n"
 "          be resolved.\n"
 "\n"
-"     6    Couldn't resolve host. The given remote host was not\n"
+);
+ puts(
+"     6    Couldn't  resolve  host.  The given remote host was not\n"
 "          resolved.\n"
 "\n"
 "     7    Failed to connect to host.\n"
 "\n"
-"     8    FTP weird server reply. The server sent data curl\n"
-);
- puts(
+"     8    FTP weird server  reply.  The  server  sent  data  curl\n"
 "          couldn't parse.\n"
 "\n"
 "     9    FTP access denied. The server denied login.\n"
 "\n"
-"     10   FTP user/password incorrect. Either one or both were\n"
+"     10   FTP  user/password  incorrect.  Either one or both were\n"
 "          not accepted by the server.\n"
 "\n"
-"     11   FTP weird PASS reply. Curl couldn't parse the reply\n"
+"     11   FTP weird PASS reply. Curl  couldn't  parse  the  reply\n"
 "          sent to the PASS request.\n"
 "\n"
-"     12   FTP weird USER reply. Curl couldn't parse the reply\n"
+);
+ puts(
+"     12   FTP  weird  USER  reply.  Curl couldn't parse the reply\n"
 "          sent to the USER request.\n"
-"     13   FTP weird PASV reply, Curl couldn't parse the reply\n"
+"     13   FTP weird PASV reply, Curl  couldn't  parse  the  reply\n"
 "          sent to the PASV request.\n"
 "\n"
-);
- puts(
-"     14   FTP weird 227 format. Curl couldn't parse the 227-line\n"
+"     14   FTP  weird 227 format. Curl couldn't parse the 227-line\n"
 "          the server sent.\n"
 "\n"
 "     15   FTP can't get host. Couldn't resolve the host IP we got\n"
 "          in the 227-line.\n"
 "\n"
-"     16   FTP can't reconnect. Couldn't connect to the host we\n"
+"     16   FTP  can't  reconnect.  Couldn't connect to the host we\n"
 "          got in the 227-line.\n"
 "\n"
-"     17   FTP couldn't set binary. Couldn't change transfer\n"
+);
+ puts(
+"     17   FTP  couldn't  set  binary.  Couldn't  change  transfer\n"
 "          method to binary.\n"
 "\n"
 "     18   Partial file. Only a part of the file was transfered.\n"
 "\n"
 "     19   FTP couldn't RETR file. The RETR command failed.\n"
 "\n"
-);
- puts(
-"     20   FTP write error. The transfer was reported bad by the\n"
+"     20   FTP  write  error. The transfer was reported bad by the\n"
 "          server.\n"
 "\n"
-"     21   FTP quote error. A quote command returned error from\n"
+"     21   FTP quote error. A quote command  returned  error  from\n"
 "          the server.\n"
 "\n"
-"     22   HTTP not found. The requested page was not found. This\n"
+"     22   HTTP  not found. The requested page was not found. This\n"
+);
+ puts(
 "          return code only appears if --fail is used.\n"
 "\n"
-"     23   Write error. Curl couldn't write data to a local\n"
+"     23   Write error.  Curl  couldn't  write  data  to  a  local\n"
 "          filesystem or similar.\n"
 "\n"
 "     24   Malformat user. User name badly specified.\n"
 "\n"
-"     25   FTP couldn't STOR file. The server denied the STOR\n"
-);
- puts(
+"     25   FTP  couldn't  STOR  file.  The  server denied the STOR\n"
 "          operation.\n"
 "\n"
 "     26   Read error. Various reading problems.\n"
 "\n"
 "     27   Out of memory. A memory allocation request failed.\n"
 "\n"
-"     28   Operation timeout. The specified time-out period was\n"
+"     28   Operation timeout. The specified  time-out  period  was\n"
+);
+ puts(
 "          reached according to the conditions.\n"
 "\n"
-"     29   FTP couldn't set ASCII. The server returned an unknown\n"
+"     29   FTP  couldn't set ASCII. The server returned an unknown\n"
 "          reply.\n"
 "\n"
 "     30   FTP PORT failed. The PORT command failed.\n"
 "\n"
 "     31   FTP couldn't use REST. The REST command failed.\n"
 "\n"
-);
- puts(
-"     32   FTP couldn't use SIZE. The SIZE command failed. The\n"
-"          command is an extension to the original FTP spec RFC\n"
+"     32   FTP couldn't use SIZE. The  SIZE  command  failed.  The\n"
+"          command  is  an  extension to the original FTP spec RFC\n"
 "          959.\n"
 "\n"
 "     33   HTTP range error. The range \"command\" didn't work.\n"
 "\n"
-"     34   HTTP post error. Internal post-request generation\n"
+);
+ puts(
+"     34   HTTP  post  error.  Internal  post-request   generation\n"
 "          error.\n"
 "\n"
 "     35   SSL connect error. The SSL handshaking failed.\n"
 "\n"
-"     36   FTP bad download resume. Couldn't continue an earlier\n"
+"     36   FTP  bad  download resume. Couldn't continue an earlier\n"
 "          aborted download.\n"
 "\n"
-"     37   FILE couldn't read file. Failed to open the file. Per�\n"
-);
- puts(
+"     37   FILE couldn't read file. Failed to open the file.  Per�\n"
 "          missions?\n"
 "\n"
 "     38   LDAP cannot bind. LDAP bind operation failed.\n"
 "\n"
 "     39   LDAP search failed.\n"
 "\n"
 "     40   Library not found. The LDAP library was not found.\n"
 "\n"
-"     41   Function not found. A required LDAP function was not\n"
+);
+ puts(
+"     41   Function  not  found.  A required LDAP function was not\n"
 "          found.\n"
 "\n"
-"     42   Aborted by callback. An application told curl to abort\n"
+"     42   Aborted by callback. An application told curl to  abort\n"
 "          the operation.\n"
 "\n"
 "     43   Internal error. A function was called with a bad param�\n"
 "          eter.\n"
 "\n"
 "     44   Internal error. A function was called in a bad order.\n"
 "\n"
-);
- puts(
-"     45   Interface error. A specified outgoing interface could\n"
+"     45   Interface error. A specified outgoing  interface  could\n"
 "          not be used.\n"
 "\n"
-"     46   Bad password entered. An error was signalled when the\n"
+"     46   Bad  password  entered. An error was signalled when the\n"
+);
+ puts(
 "          password was entered.\n"
 "\n"
-"     47   Too many redirects. When following redirects, curl hit\n"
+"     47   Too many redirects. When following redirects, curl  hit\n"
 "          the maximum amount.\n"
 "\n"
-"     XX   There will appear more error codes here in future\n"
-"          releases. The existing ones are meant to never change.\n"
+"     XX   There  will  appear  more  error  codes  here in future\n"
+"          releases. The existing ones are meant to never  change.\n"
 "\n"
 "BUGS\n"
 "     If you do find bugs, mail them to curl-bug@haxx.se.\n"
 "\n"
 "AUTHORS / CONTRIBUTORS\n"
-);
- puts(
 "      - Daniel Stenberg <Daniel.Stenberg@haxx.se>\n"
 "      - Rafael Sagula <sagula@inf.ufrgs.br>\n"
 "      - Sampo Kellomaki <sampo@iki.fi>\n"
+);
+ puts(
 "      - Linas Vepstas <linas@linas.org>\n"
 "      - Bjorn Reese <breese@mail1.stofanet.dk>\n"
 "      - Johan Anderson <johan@homemail.com>\n"
 "      - Kjell Ericson <Kjell.Ericson@haxx.se>\n"
 "      - Troy Engel <tengel@sonic.net>\n"
 "      - Ryan Nelson <ryan@inch.com>\n"
 "      - Bj�rn Stenberg <Bjorn.Stenberg@haxx.se>\n"
 "      - Angus Mackay <amackay@gus.ml.org>\n"
-);
- puts(
 "      - Eric Young <eay@cryptsoft.com>\n"
 "      - Simon Dick <simond@totally.irrelevant.org>\n"
 "      - Oren Tirosh <oren@monty.hishome.net>\n"
+);
+ puts(
 "      - Steven G. Johnson <stevenj@alum.mit.edu>\n"
 "      - Gilbert Ramirez Jr. <gram@verdict.uthscsa.edu>\n"
 "      - Andr�s Garc�a <ornalux@redestb.es>\n"
 "      - Douglas E. Wegscheid <wegscd@whirlpool.com>\n"
 "      - Mark Butler <butlerm@xmission.com>\n"
 "      - Eric Thelin <eric@generation-i.com>\n"
 "      - Marc Boucher <marc@mbsi.ca>\n"
-);
- puts(
 "      - Greg Onufer <Greg.Onufer@Eng.Sun.COM>\n"
 "      - Doug Kaufman <dkaufman@rahul.net>\n"
 "      - David Eriksson <david@2good.com>\n"
 "      - Ralph Beckmann <rabe@uni-paderborn.de>\n"
+);
+ puts(
 "      - T. Yamada <tai@imasy.or.jp>\n"
 "      - Lars J. Aas <larsa@sim.no>\n"
 "      - J�rn Hartroth <Joern.Hartroth@computer.org>\n"
 "      - Matthew Clarke <clamat@van.maves.ca>\n"
 "      - Linus Nielsen Feltzing <linus@haxx.se>\n"
 "      - Felix von Leitner <felix@convergence.de>\n"
 "      - Dan Zitter <dzitter@zitter.net>\n"
-);
- puts(
 "      - Jongki Suwandi <Jongki.Suwandi@eng.sun.com>\n"
 "      - Chris Maltby <chris@aurema.com>\n"
 "      - Ron Zapp <rzapper@yahoo.com>\n"
 "      - Paul Marquis <pmarquis@iname.com>\n"
+);
+ puts(
 "      - Ellis Pritchard <ellis@citria.com>\n"
 "      - Damien Adant <dams@usa.net>\n"
 "      - Chris <cbayliss@csc.come>\n"
 "      - Marco G. Salvagno <mgs@whiz.cjb.net>\n"
 "      - Paul Marquis <pmarquis@iname.com>\n"
 "      - David LeBlanc <dleblanc@qnx.com>\n"
 "      - Rich Gray at Plus Technologies\n"
 "      - Luong Dinh Dung <u8luong@lhsystems.hu>\n"
-);
- puts(
 "      - Torsten Foertsch <torsten.foertsch@gmx.net>\n"
 "      - Kristian K�hntopp <kris@koehntopp.de>\n"
 "      - Fred Noz <FNoz@siac.com>\n"
+);
+ puts(
 "      - Caolan McNamara <caolan@csn.ul.ie>\n"
 "      - Albert Chin-A-Young <china@thewrittenword.com>\n"
 "      - Stephen Kick <skick@epicrealm.com>\n"
 "      - Martin Hedenfalk <mhe@stacken.kth.se>\n"
 "      - Richard Prescott\n"
 "      - Jason S. Priebe <priebe@wral-tv.com>\n"
 "      - T. Bharath <TBharath@responsenetworks.com>\n"
 "      - Alexander Kourakos <awk@users.sourceforge.net>\n"
-);
- puts(
 "      - James Griffiths <griffiths_james@yahoo.com>\n"
 "      - Loic Dachary <loic@senga.org>\n"
+);
+ puts(
 "      - Robert Weaver <robert.weaver@sabre.com>\n"
 "      - Ingo Ralf Blum <ingoralfblum@ingoralfblum.com>\n"
 "      - Jun-ichiro itojun Hagino <itojun@iijlab.net>\n"
 "\n"
 "WWW\n"
 "     http://curl.haxx.se\n"
