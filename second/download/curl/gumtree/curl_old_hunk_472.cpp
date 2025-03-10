"              ftp_entry_path The initial path libcurl ended up in when logging\n"
"                             on to the remote FTP server. (Added in 7.15.4)\n"
"\n"
"              ssl_verify_result\n"
, stdout);
 fputs(
"                             The  result of the SSL peer certificate verifica-\n"
"                             tion that was requested. 0 means the verification\n"
"                             was successful. (Added in 7.19.0)\n"
"\n"
"       If this option is used several times, the last one will be used.\n"
"\n"
"       -x/--proxy <proxyhost[:port]>\n"
"              Use  the  specified HTTP proxy. If the port number is not speci-\n"
"              fied, it is assumed at port 1080.\n"
"\n"
, stdout);
 fputs(
"              This option overrides existing environment  variables  that  set\n"
"              the  proxy  to use. If there's an environment variable setting a\n"
"              proxy, you can set proxy to \"\" to override it.\n"
"\n"
"              Note that all operations that are performed over  a  HTTP  proxy\n"
"              will  transparently  be converted to HTTP. It means that certain\n"
"              protocol specific operations might not be available. This is not\n"
, stdout);
 fputs(
"              the  case  if you can tunnel through the proxy, as done with the\n"
"              -p/--proxytunnel option.\n"
"\n"
"              Starting with 7.14.1, the proxy host can be specified the  exact\n"
"              same  way as the proxy environment variables, including the pro-\n"
"              tocol prefix (http://) and the embedded user + password.\n"
"\n"
"              If this option is used several times, the last one will be used.\n"
"\n"
"       -X/--request <command>\n"
, stdout);
 fputs(
"              (HTTP) Specifies a custom request method to use when communicat-\n"
"              ing  with  the  HTTP server.  The specified request will be used\n"
"              instead of the method otherwise used (which  defaults  to  GET).\n"
"              Read  the  HTTP  1.1 specification for details and explanations.\n"
"              Common additional HTTP requests  include  PUT  and  DELETE,  but\n"
"              related technologies like WebDAV offers PROPFIND, COPY, MOVE and\n"
"              more.\n"
"\n"
, stdout);
 fputs(
"              (FTP) Specifies a custom FTP command to use instead of LIST when\n"
