, stdout);
 fputs(
"              (HTTP) This option tells the -O, --remote-name option to use the\n"
"              server-specified   Content-Disposition   filename   instead   of\n"
"              extracting a filename from the URL.\n"
"\n"
"              There's  no  attempt to decode %-sequences (yet) in the provided\n"
"              file name, so this option may provide you with rather unexpected\n"
"              file names.\n"
"\n"
"       -k, --insecure\n"
"              (SSL)  This  option explicitly allows curl to perform \"insecure\"\n"
, stdout);
 fputs(
"              SSL connections and transfers. All SSL connections are attempted\n"
"              to  be  made secure by using the CA certificate bundle installed\n"
"              by default. This makes  all  connections  considered  \"insecure\"\n"
"              fail unless -k, --insecure is used.\n"
"\n"
"              See     this    online    resource    for    further    details:\n"
"              http://curl.haxx.se/docs/sslcerts.html\n"
"\n"
"       -K, --config <config file>\n"
, stdout);
 fputs(
"              Specify which config file to read curl arguments from. The  con-\n"
"              fig  file  is a text file in which command line arguments can be\n"
"              written which then will be used as if they were written  on  the\n"
"              actual command line.\n"
"\n"
"              Options  and their parameters must be specified on the same con-\n"
"              fig file line, separated by whitespace,  colon,  or  the  equals\n"
, stdout);
 fputs(
"              sign.  Long  option  names can optionally be given in the config\n"
"              file without the initial double dashes and if so, the  colon  or\n"
"              equals  characters  can  be used as separators. If the option is\n"
"              specified with one or two dashes,  there  can  be  no  colon  or\n"
"              equals character between the option and its parameter.\n"
"\n"
"              If the parameter is to contain whitespace, the parameter must be\n"
, stdout);
 fputs(
"              enclosed within quotes.  Within  double  quotes,  the  following\n"
"              escape  sequences  are  available:  \\\\, \\\", \\t, \\n, \\r and \\v. A\n"
"              backslash preceding any other letter is ignored.  If  the  first\n"
"              column of a config line is a '#' character, the rest of the line\n"
"              will be treated as a comment. Only write one option per physical\n"
"              line in the config file.\n"
"\n"
, stdout);
 fputs(
"              Specify  the  filename  to -K, --config as '-' to make curl read\n"
"              the file from stdin.\n"
"\n"
"              Note that to be able to specify a URL in the  config  file,  you\n"
"              need  to  specify  it  using the --url option, and not by simply\n"
"              writing the URL on its own line. So, it could  look  similar  to\n"
"              this:\n"
"\n"
"              url = \"http://curl.haxx.se/docs/\"\n"
"\n"
, stdout);
 fputs(
"              When curl is invoked, it always (unless -q is used) checks for a\n"
"              default config file and uses it if  found.  The  default  config\n"
"              file is checked for in the following places in this order:\n"
"\n"
"              1)  curl  tries  to find the \"home dir\": It first checks for the\n"
"              CURL_HOME and then the HOME environment variables. Failing that,\n"
"              it  uses getpwuid() on UNIX-like systems (which returns the home\n"
, stdout);
 fputs(
"              dir given the current user in your system). On Windows, it  then\n"
"              checks for the APPDATA variable, or as a last resort the '%USER-\n"
"              PROFILE%\\Application Data'.\n"
"\n"
"              2) On windows, if there is no _curlrc file in the home  dir,  it\n"
"              checks for one in the same dir the curl executable is placed. On\n"
"              UNIX-like systems, it will simply try to load .curlrc  from  the\n"
"              determined home dir.\n"
"\n"
, stdout);
 fputs(
"              # --- Example file ---\n"
"              # this is a comment\n"
