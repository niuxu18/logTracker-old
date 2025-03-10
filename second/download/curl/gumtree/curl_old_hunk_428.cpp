"\n"
"       -y/--speed-time <time>\n"
"              If a download is slower than speed-limit bytes per second during\n"
"              a speed-time period, the download gets aborted. If speed-time is\n"
"              used, the default speed-limit will be 1 unless set with -Y.\n"
"\n"
"              This  option  controls  transfers  and thus will not affect slow\n"
, stdout);
 fputs(
"              connects etc. If this is a concern for you, try  the  --connect-\n"
"              timeout option.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -Y/--speed-limit <speed>\n"
"              If a download is slower than this given speed (in bytes per sec-\n"
"              ond) for speed-time seconds it gets aborted. speed-time  is  set\n"
"              with -y and is 30 if not set.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -z/--time-cond <date expression>\n"
"              (HTTP/FTP)  Request a file that has been modified later than the\n"
"              given time and date, or one that has been modified  before  that\n"
"              time. The date expression can be all sorts of date strings or if\n"
"              it doesn't match any internal ones, it tries  to  get  the  time\n"
, stdout);
 fputs(
"              from  a  given  file  name  instead! See the curl_getdate(3) man\n"
"              pages for date expression details.\n"
"\n"
"              Start the date expression with a dash (-) to make it request for\n"
"              a  document that is older than the given date/time, default is a\n"
"              document that is newer than the specified date/time.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       --max-redirs <num>\n"
, stdout);
 fputs(
"              Set  maximum  number  of  redirection-followings   allowed.   If\n"
"              -L/--location  is  used, this option can be used to prevent curl\n"
"              from following redirections \"in absurdum\". By default, the limit\n"
"              is set to 50 redirections. Set this option to -1 to make it lim-\n"
"              itless.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -0/--http1.0\n"
, stdout);
 fputs(
"              (HTTP) Forces curl to issue its requests using HTTP 1.0  instead\n"
"              of using its internally preferred: HTTP 1.1.\n"
"\n"
"       -1/--tlsv1\n"
"              (SSL)  Forces  curl to use TLS version 1 when negotiating with a\n"
"              remote TLS server.\n"
"\n"
"       -2/--sslv2\n"
"              (SSL) Forces curl to use SSL version 2 when negotiating  with  a\n"
"              remote SSL server.\n"
"\n"
"       -3/--sslv3\n"
"              (SSL)  Forces  curl to use SSL version 3 when negotiating with a\n"
, stdout);
 fputs(
"              remote SSL server.\n"
"\n"
"       -4/--ipv4\n"
"              If libcurl is capable of resolving an  address  to  multiple  IP\n"
"              versions  (which it is if it is IPv6-capable), this option tells\n"
"              libcurl to resolve names to IPv4 addresses only.\n"
"\n"
"       -6/--ipv6\n"
"              If libcurl is capable of resolving an  address  to  multiple  IP\n"
"              versions  (which it is if it is IPv6-capable), this option tells\n"
, stdout);
 fputs(
"              libcurl to resolve names to IPv6 addresses only.\n"
"\n"
"       -#/--progress-bar\n"
"              Make curl display progress information as a progress bar instead\n"
