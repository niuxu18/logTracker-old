 "              a GET if the HTTP response was 301, 302, or 303. If the response\n"
 "              code  was  any  other  3xx code, curl will re-send the following\n"
 "              request using the same unmodified method.\n"
 "\n"
 "       --libcurl <file>\n"
 "              Append this option to any ordinary curl command  line,  and  you\n"
-"              will  get  a  libcurl-using source code written to the file that\n"
+"              will  get a libcurl-using C source code written to the file that\n"
 "              does the equivalent of what your command-line operation does!\n"
 "\n"
 , stdout);
  fputs(
-"              NOTE: this does not properly support -F and the sending of  mul-\n"
-"              tipart  formposts,  so in those cases the output program will be\n"
-"              missing necessary calls to curl_formadd(3), and possibly more.\n"
-"\n"
 "              If this option is used several times, the last given  file  name\n"
 "              will be used. (Added in 7.16.1)\n"
 "\n"
 "       --limit-rate <speed>\n"
 "              Specify  the  maximum  transfer  rate you want curl to use. This\n"
-, stdout);
- fputs(
 "              feature is useful if you have a limited pipe and you'd like your\n"
 "              transfer not to use your entire bandwidth.\n"
 "\n"
 "              The  given speed is measured in bytes/second, unless a suffix is\n"
+, stdout);
+ fputs(
 "              appended.  Appending 'k' or 'K' will count the number  as  kilo-\n"
 "              bytes,  'm'  or M' makes it megabytes, while 'g' or 'G' makes it\n"
 "              gigabytes. Examples: 200K, 3m and 1G.\n"
 "\n"
-, stdout);
- fputs(
 "              The given rate is the average speed counted  during  the  entire\n"
 "              transfer. It means that curl might use higher transfer speeds in\n"
 "              short bursts, but over time it uses no more than the given rate.\n"
+, stdout);
+ fputs(
 "              If you also use the -Y, --speed-limit option, that  option  will\n"
 "              take precedence and might cripple the rate-limiting slightly, to\n"
 "              help keeping the speed-limit logic working.\n"
 "\n"
-, stdout);
- fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
 "       --local-port <num>[-num]\n"
 "              Set a preferred number or range of local port numbers to use for\n"
 "              the  connection(s).   Note  that  port  numbers  by nature are a\n"
+, stdout);
+ fputs(
 "              scarce resource that will be busy at times so setting this range\n"
 "              to something too narrow might cause unnecessary connection setup\n"
 "              failures. (Added in 7.15.2)\n"
 "\n"
 "       --location-trusted\n"
-, stdout);
- fputs(
 "              (HTTP/HTTPS) Like -L, --location, but  will  allow  sending  the\n"
 "              name + password to all hosts that the site may redirect to. This\n"
 "              may or may not introduce a security breach if the site redirects\n"
+, stdout);
+ fputs(
 "              you  to  a  site  to  which you'll send your authentication info\n"
 "              (which is plaintext in the case of HTTP Basic authentication).\n"
 "\n"
 "       -m, --max-time <seconds>\n"
-, stdout);
- fputs(
 "              Maximum time in seconds that you allow the  whole  operation  to\n"
 "              take.   This is useful for preventing your batch jobs from hang-\n"
 "              ing for hours due to slow networks or  links  going  down.   See\n"
 "              also the --connect-timeout option.\n"
 "\n"
+, stdout);
+ fputs(
 "              If this option is used several times, the last one will be used.\n"
 "\n"
+"       --mail-auth <address>\n"
+"              (SMTP)  Specify  a  single address. This will be used to specify\n"
+"              the authentication address (identity)  of  a  submitted  message\n"
+"              that is being relayed to another server.\n"
+"\n"
+"              (Added in 7.25.0)\n"
+"\n"
 "       --mail-from <address>\n"
 "              (SMTP)  Specify  a single address that the given mail should get\n"
 "              sent from.\n"
+"\n"
 , stdout);
  fputs(
-"\n"
 "              (Added in 7.20.0)\n"
 "\n"
 "       --max-filesize <bytes>\n"
 "              Specify the maximum size (in bytes) of a file  to  download.  If\n"
 "              the  file requested is larger than this value, the transfer will\n"
 "              not start and curl will return with exit code 63.\n"
