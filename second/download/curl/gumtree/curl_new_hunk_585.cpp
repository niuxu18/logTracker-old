"\n"
"       -y, --speed-time <time>\n"
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
"       -Y, --speed-limit <speed>\n"
"              If a download is slower than this given speed (in bytes per sec-\n"
"              ond) for speed-time seconds it gets aborted. speed-time  is  set\n"
"              with -y and is 30 if not set.\n"
"\n"
, stdout);
 fputs(
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -z, --time-cond <date expression>\n"
"              (HTTP/FTP/FILE) Request a file that has been modified later than\n"
"              the given time and date, or one that has  been  modified  before\n"
"              that  time. The date expression can be all sorts of date strings\n"
"              or if it doesn't match any internal ones, it tries  to  get  the\n"
, stdout);
 fputs(
"              time from a given file name instead! See the curl_getdate(3) man\n"
"              pages for date expression details.\n"
"\n"
"              Start the date expression with a dash (-) to make it request for\n"
"              a  document that is older than the given date/time, default is a\n"
"              document that is newer than the specified date/time.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -h, --help\n"
"              Usage help.\n"
